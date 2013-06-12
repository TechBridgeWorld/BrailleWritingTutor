#ifndef ARITHMETIC_H_
#define ARITHMETIC_H_

#include "common/IBTApp.h"
#include "common/language_utils.h"
#include "common/utilities.h"

#define FRACTION_MODE 0 // boolean value indicating fractions mode

 // will be set to one of the four following
#define ADDITION 1
#define SUBTRACTION 2
#define MULTIPLICATION 3
#define DIVISION 4
#define MAX_DIGITS 6

class Arithmetic : public IBTApp
{
public:
	// added string to hold number mapping file path
  explicit Arithmetic(IOEventParser&, const std::string&, SoundsUtil*, bool);
  ~Arithmetic();

  void processEvent(IOEvent& event);

private:
  void Fact_new();
  void AP_attempt(unsigned char);
  void sayArithmeticQuestion(bool);
  void getDigits(int, int*);
  void say_multidigit(int*);
  void clearArray(int*);

  SoundsUtil* su;
  const Voice math_s;
  bool nomirror;
  DotSequence target_sequence;//the dot sequence of the target number(between 0-9)
  DotSequence current_sequence;//holds the on-going dot-sequence the user is entering..
  int n1;
  int n2;
  int num1;
  int num2;
  int result; 
  int digit, i, current_target;
  DotSequence dots1;
  DotSequence dots2;
  int response_array[MAX_DIGITS];
  int num1_array[MAX_DIGITS];
  int num2_array[MAX_DIGITS];
 
};



class EnglishArithmeticPractice : public Arithmetic
{
public:
  explicit EnglishArithmeticPractice(IOEventParser&);
  ~EnglishArithmeticPractice()
  {

  }
};

/* put in the rest here */


#endif /* ARITHMETIC_H_ */