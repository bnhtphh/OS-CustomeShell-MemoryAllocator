/**
 * @file broken.c
 * @author mmalensek
 *
 * This program contains a series of buggy, broken, or strange C functions for
 * you to ponder. Your job is to analyze each function, fix whatever bugs the
 * functions might have, and then explain what went wrong. Sometimes the
 * compiler will give you a hint.
 *
 *  ____________
 * < Good luck! >
 *  ------------
 *      \   ^__^
 *       \  (oo)\_______
 *          (__)\       )\/\
 *              ||----w |
 *              ||     ||
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int func_counter = 1;
#define FUNC_START() printf("\n\n%d.) %s\n", func_counter++, __func__);

#pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#pragma GCC diagnostic ignored "-Wformat-overflow"

/**
 * This code example was taken from the book 'Mastering C Pointers,' one of the
 * not so good ways to learn pointers. It was trying to demonstrate how to print
 * 'BNGULAR'... with pointers...? Maybe?
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *      Hint: where are string literals stored in memory?
 *      Hint: what is '66' in this example? Can we do better?

*/

void
angular(void)
{
  FUNC_START();

  char a[] = "ANGULAR";
  a[0] = 66; //Not pointer, using array
  printf("%s\n", a);
}


 // ---------------------------------------------------------------------------------------//

 
/**
 * This function is the next step after 'Hello world' -- it takes user input and
 * prints it back out! (Wow).
 *
 * But, unfortunately, it doesn't work.
 * --less 7 on gets(name, 127) for safe 
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   (answer) allocate buffer
 */
void
greeter(void)
{
  FUNC_START();

  char name[128] = ""; 
//  char *ptr = name;

  printf("Please enter your name: ");
  gets(name, 127);

  // Remove newline character
  char *p = name;
  for ( ; *p != '\n' && *p != 0; p++) { }
  *p = '\0';

  printf("Hello, %s!\n", name);
}


// ---------------------------------------------------------------------------------------//

 
/**
 * This code isn't so much broken, but more of an exercise in understanding how
 * C data types are represented in memory.
 *
 * (1) Fill in your guesses below to see if you can get all 6 correct on the
 *     first try.
 * (2) Explain *WHY* you get the sizes you do for each of the 6 entries.
 *     sizeof(int) and sizeof(float) are fairly straightforward.
 *
 */
#define SIZE_CHECK(sz, guess) (sz), sz == guess ? "Right!" : "Wrong!"
void
sizer(void)
{
  FUNC_START();

  int guesses[] = { 1, 4, 4, 48, 4, 2 }; // fill in 6 guesses here
  if (sizeof(guesses) / sizeof(int) != 6) {
    printf("Please enter a guess for each of the sizes below.\n");
    printf("Example: guesses[] = { 1, 4, 0, 0, 0, 0 }\n");
    printf("would mean sizeof(char) == 1, sizeof(int) == 4, and so on");
    return;
  }

  int things[12] = { 0 };
  // for char, it only contain or store a single character, and it is 1 byte (8 bits)
  printf("sizeof(char)   = %ld\t[%s]\n", SIZE_CHECK(sizeof(char), guesses[0])); 
  // for int, it's mean integer, it stote a number, which is 4 bytes
  printf("sizeof(int)    = %ld\t[%s]\n", SIZE_CHECK(sizeof(int), guesses[1]));
  // float is floating-point which is store decimal number, like int, 4 bytes
  printf("sizeof(float)  = %ld\t[%s]\n", SIZE_CHECK(sizeof(float), guesses[2]));
  // array things have size 12, and it is int array, so it is 4 byte for int, 4 * 12 = 48
  printf("sizeof(things) = %ld\t[%s]\n", SIZE_CHECK(sizeof(things), guesses[3]));
  // character in c, known by int 
  printf("sizeof('A')    = %ld\t[%s]\n", SIZE_CHECK(sizeof('A'), guesses[4]));
  // 1 byte and null byte
  printf("sizeof(\"A\")    = %ld\t[%s]\n", SIZE_CHECK(sizeof("A"), guesses[5]));
}
// ---------------------------------------------------------------------------------------//
/**
 * This 'useful' function prints out an array of integers with their indexes, or
 * at least tries to. It even has a special feature where it adds '12' to the
 * array.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   (answer) Array index and size of loop for
 */
void
displayer(void)
{
  FUNC_START();

  int stuff[100] = { 0 };

  // Can you guess what the following does without running the program? 
  //  Rewrite it so it's easier to read. 
  //14[stuff + 1] = 12;
  stuff[15] = 12;

  for (int i = 0; i < 100; ++i) {
    printf("%d: %d\n", i, stuff[i]);
  }
}


// ---------------------------------------------------------------------------------------//
/**
 * Adds up the contents of an array and prints the total. Unfortunately the
 * total is wrong! See main() for the arguments that were passed in.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   (answer) sizeof(arr) is give the size of pointer
 */
void
adder(int *arr)
{
  FUNC_START();

  int total = 0;

  for (int i = 0; i < 10; ++i) {
    total += arr[i];
  }

  printf("Total is: %d\n", total);
}


// ---------------------------------------------------------------------------------------//

/**
 * This function is supposed to be somewhat like strcat, but it doesn't work.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   (answer) the memory of buf
 */

char *
suffixer(char *a, char *b)
{
  FUNC_START();

  int len_a = strlen(a);
  int len_b = strlen(b);
  char *buf = malloc(len_a + len_b + 1); // Dynamic allocation 
 // char buf[128] = { 0 };

  char *buf_start = buf;
  strcpy(buf, a);
  strcpy(buf + strlen(a), b);
  return buf_start;

}


// ---------------------------------------------------------------------------------------//

/**
 * This is an excerpt of Elon Musk's Full Self Driving code. Unfortunately, it
 * keeps telling people to take the wrong turn. Figure out how to fix it, and
 * explain what's going on so Elon can get back to work on leaving Earth for
 * good.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 * overflow issue is size of character and strcomp 
 *   (answer) size is not match and condition for compare 
*/
void
driver(void)
{
  FUNC_START();

  char street1[8] = { "fulton" };
  char street2[8] = { "gomery" };
  // changing the size for it 
  char street3[9] = { "piedmont" };
  char street4[8] = { "baker" };
  char street5[8] = { "haight" };

// need to have a conditional for compare
  if (strcmp(street1, street2) == 0) {
    char *new_name = "saint agnew ";
    memcpy(street4, new_name, strlen(new_name));
  }

  printf("Welcome to TeslaOS 0.1!\n");
  printf("Calculating route...\n");
  printf("Turn left at the intersection of %s and %s.\n", street5, street3);
}

// ---------------------------------------------------------------------------------------//


/**
 * This function tokenizes a string by space, sort of like a basic strtok or
 * strsep. It has two subtle memory bugs for you to find.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   (answer) malloc add 1 for null terminator
*/
void
tokenizer(void)
{
  FUNC_START();

  char *str = "Hope was a letter I never could send";
  char *line = malloc(strlen(str) + 1);
  char *c = line;

  strcpy(line, str);

  while (*c != '\0') {

    for ( ; *c != ' ' && *c != '\0'; c++) {
      // find the next space (or end of string)
    }

    *c = '\0';
    printf("%s\n", line);

    line = c + 1;
    c = line;
  }

  free(line);
}


// ---------------------------------------------------------------------------------------//
/**
* This function should print one thing you like about C, and one thing you
* dislike about it. Assuming you don't mess up, there will be no bugs to find
* here!
*/
void
finisher(void)
{
  FUNC_START();

  // TODO
  printf("I can understand C as it is usually written\n");
  printf("Sometime i hate globle variable\n");
}

int
main(void)
{
  printf("Starting up!");


  angular();

  greeter();

  sizer();

  displayer();

  int nums[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };
  adder(nums);

  char *result = suffixer("kayak", "ing");
  printf("Suffixed: %s\n", result);

  driver();

  tokenizer();
  finisher();

  return 0;
}

