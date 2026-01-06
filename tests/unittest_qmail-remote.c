#include <check.h>

#include "substdio.h"

/* provided by qmail-remote.c/blast.c */
extern void blast();
extern char inbuf[1024];
extern substdio ssin;
extern char smtptobuf[1024];
extern substdio smtpto;

static const char *readexpect;
static const char *writeexpect;
static size_t readoffs;
static size_t writeoffs;

ssize_t readstub(int fd, char *buf, size_t len)
{
  size_t inlen = strlen(readexpect) - readoffs;

  ck_assert_int_eq(fd, -1);
  ck_assert_int_gt(len, 0);

  if (len < inlen)
    inlen = len;

  if (inlen > 0) {
    memcpy(buf, readexpect + readoffs, inlen);
    readoffs += inlen;
  }

  return inlen;
}

ssize_t writestub(int fd, const char *buf, size_t len)
{
  size_t outlen = strlen(writeexpect) - writeoffs;

  ck_assert_int_eq(fd, -1);
  ck_assert_int_gt(len, 0);

  if (len < outlen)
    outlen = len;

#if (CHECK_MAJOR_VERSION > 0) || (CHECK_MINOR_VERSION >= 11)
  ck_assert_mem_eq(writeexpect + writeoffs, buf, outlen);
#else
  ck_assert_int_eq(memcmp(writeexpect + writeoffs, buf, outlen), 0);
#endif

  writeoffs += outlen;

  return outlen;
}

static void ssin_setup(const char *indata, const char *outdata)
{
  substdio tmpin = SUBSTDIO_FDBUF(readstub,-1,inbuf,sizeof(inbuf));
  substdio tmpto = SUBSTDIO_FDBUF(writestub,-1,smtptobuf,sizeof(smtptobuf));
  ssin = tmpin;
  smtpto = tmpto;

  readexpect = indata;
  readoffs = 0;
  writeexpect = outdata;
  writeoffs = 0;
}

START_TEST(test_blast_empty)
{
  const char *dotcrlf = ".\r\n";

  ssin_setup("", dotcrlf);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf));
}
END_TEST

START_TEST(test_blast_dot)
{
  const char *dotcrlf = "..\r\n.\r\n";

  ssin_setup(".\n", dotcrlf);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf));
}
END_TEST

START_TEST(test_blast_barecr)
{
  const char *dotcrlf = "cr\r\nlf\r\n.\r\n";

  ssin_setup("cr\rlf\n", dotcrlf);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf));
}
END_TEST

START_TEST(test_blast_barecr_4k)
{
  /* Check correct operation when bare CR is in last position of buffer.
     Assumes 4k buffer size in "blast" */
  char dotcrlf_4k[4095 + 9 + 1];
  char barecr_4k[4095 + 5 + 1];

  char ch = '0';
  memset(dotcrlf_4k, 0, sizeof(dotcrlf_4k));
  memset(barecr_4k, 0, sizeof(barecr_4k));
  for (int i = 0; i < 4095; i++) {
    barecr_4k[i] = ch;
    dotcrlf_4k[i] = ch;
    ch = (ch == '9' ? '0' : ++ch);
  }
  strcat(barecr_4k, "\rlf\n");
  ck_assert_uint_eq( strlen(barecr_4k), 4099 );
  strcat(dotcrlf_4k, "\r\nlf\r\n.\r\n");
  ck_assert_uint_eq( strlen(dotcrlf_4k), 4104 );

  ssin_setup(barecr_4k, dotcrlf_4k);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf_4k));
}
END_TEST

START_TEST(test_blast_crlf)
{
  const char *dotcrlf = "..\r\n.\r\n";

  ssin_setup(".\r\n", dotcrlf);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf));
}
END_TEST

START_TEST(test_blast_crlf_4k)
{
  /* Check correct operation when CR+LF is split across buffer boundary.
     Assumes 4k buffer size in "blast" */
  char dotcrlf_4k[4095 + 5 + 1];
  char crlf_4k[4095 + 2 + 1];

  char ch = '0';
  memset(dotcrlf_4k, 0, sizeof(dotcrlf_4k));
  memset(crlf_4k, 0, sizeof(crlf_4k));
  for (int i = 0; i < 4095; i++) {
    crlf_4k[i] = ch;
    dotcrlf_4k[i] = ch;
    ch = (ch == '9' ? '0' : ++ch);
  }
  strcat(crlf_4k, "\r\n");
  ck_assert_uint_eq( strlen(crlf_4k), 4097 );
  strcat(dotcrlf_4k, "\r\n.\r\n");
  ck_assert_uint_eq( strlen(dotcrlf_4k), 4100 );

  ssin_setup(crlf_4k, dotcrlf_4k);

  blast();

  ck_assert_uint_eq(writeoffs, strlen(dotcrlf_4k));
}
END_TEST

TCase
*blast_something(void)
{
  TCase *tc = tcase_create("basic operations");

  tcase_add_test(tc, test_blast_empty);
  tcase_add_test(tc, test_blast_dot);
  tcase_add_test(tc, test_blast_barecr);
  tcase_add_test(tc, test_blast_barecr_4k);
  tcase_add_test(tc, test_blast_crlf);
  tcase_add_test(tc, test_blast_crlf_4k);

  return tc;
}

Suite
*blast_suite(void)
{
  Suite *s = suite_create("notqmail qmail-remote blast");

  suite_add_tcase(s, blast_something());

  return s;
}

int
main(void)
{
  int number_failed;

  SRunner *sr = srunner_create(blast_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
