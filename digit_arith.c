// digit-by-digit arithmetic for HP calculator emulation
// SPDX-License-Identifier: MIT License

// Copyright © 1995-2023 Eric Smith
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice (including
// the next paragraph) shall be included in all copies or substantial
// portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <inttypes.h>
#include <stdbool.h>

static const uint64_t zero = 0;

uint64_t add_sub(bool bcd,
		 bool subtract,
		 uint64_t arg1,
		 uint64_t arg2,  // for subtraction, subtrahend
		 bool carry_in,
		 int low_digit_idx,
		 int high_digit_idx,
		 bool* carry_out)
{
  uint64_t result = arg1;
  bool carry = carry_in ^ subtract;  // for subtract, treat carry flag as borrow

  for (int digit_idx = low_digit_idx; digit_idx  <= high_digit_idx; digit_idx++)
  {
    int shift_count = digit_idx * 4;
    uint64_t mask = 0xfULL << shift_count;
    int arg1_digit = (arg1 & mask) >> shift_count;
    int arg2_digit = (arg2 & mask) >> shift_count;
    if (subtract)
      arg2_digit ^= 0xf;
    int sum_digit = arg1_digit + arg2_digit + carry;
    carry = sum_digit > 0xf;
    if (bcd)
    {
      if (subtract)
      {
	if (! carry)
	  sum_digit += 10;
      }
      else
      {
	carry |= (sum_digit > 9);
	if (carry)
	  sum_digit += 6;
      }
    }
    result = (result & ~mask) | ((sum_digit & 0xfULL) << shift_count);
  }

  if (carry_out)
    *carry_out = carry ^ subtract;  // for subtract, treat carry flag as borrow;
  return result;
}

uint64_t add_one_bcd_m(uint64_t arg)
{
  return add_sub(true,        // bcd
		 false,       // subtract
		 arg,         // arg1
		 zero,        // arg2
		 1,           // carry in
		 3, 12,       // digit indices
		 (void *) 0); // carry_out
}

uint64_t add_one_bcd_x(uint64_t arg)
{
  return add_sub(true,        // bcd
		 false,       // subtract
		 arg,         // arg1
		 zero,        // arg2
		 1,           // carry in
		 0, 1,        // digit indices
		 (void *) 0); // carry_out
}


// everything below is a main program to perform unit tests
// (presently only for BCD increment)

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  uint64_t arg;
  uint64_t expected_result_m;
  uint64_t expected_result_x;
} test_case_t;


// TODO: add more unit tests
static const test_case_t test_cases[] =
{
  { 0x00000000000000ULL, 0x00000000001000ULL, 0x00000000000001ULL },
  { 0x00000000009009ULL, 0x00000000010009ULL, 0x00000000009010ULL },
  { 0x0000000000a00aULL, 0x0000000001100aULL, 0x0000000000a011ULL },
  { 0xf006a0000000abULL, 0xf00700000010abULL, 0xf006a000000012ULL },
};

static const int test_case_count = sizeof(test_cases) / sizeof(test_case_t);


int main(int argc, char **argv)
{
  if (argc != 1)
  {
    fprintf(stderr, "%s takes no command line arguments.\n", argv[0]);
    exit(1);
  }

  int fail_count = 0;
  printf("%-14s %-14s %-14s %-14s %-14s\n", "arg", "exp result m", "result m", "exp result x", "result x");
  printf("-------------- -------------- -------------- -------------- --------------\n");
  for (int case_num = 0; case_num < test_case_count; case_num++)
  {
    const test_case_t* test_case = &test_cases[case_num];
    bool fail = false;
    uint64_t result_m = add_one_bcd_m(test_case->arg);
    uint64_t result_x = add_one_bcd_x(test_case->arg);
    printf("%014" PRIx64 " %014" PRIx64 " ", test_case->arg, test_case->expected_result_m);
    if (result_m == test_case->expected_result_m)
    {
      printf("%14s", "");
    }
    else
    {
      printf("%014" PRIx64, result_m);
      fail = true;
    }
    printf(" %014" PRIx64 " ", test_case->expected_result_x);
    if (result_x == test_case->expected_result_x)
    {
      printf("%14s", "");
    }
    else
    {
      printf("%014" PRIx64, result_x);
      fail = true;
    }
    if (fail)
    {
      fail_count++;
    }
    printf("\n");
  }

  printf("\n%d test cases have failure(s).\n", fail_count);
}
