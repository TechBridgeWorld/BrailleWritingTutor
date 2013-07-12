#ifndef HOUSEHOLD_H_
#define HOUSEHOLD_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/KnowledgeTracer.h"
#include "common/language_utils.h"

//maps the actual names of the animals in a foreign (ie,non-native) language to their english names
  typedef std::map<std::string,std::string> ForeignLanguage2EnglishMap;
  
  //not really for animals, this is for household version of animal game
class Household: public IBTApp
{
public:
  explicit Household(IOEventParser&, const std::string&, SoundsUtil*, const std::vector<std::string>, const ForeignLanguage2EnglishMap, const ForeignLanguage2EnglishMap, const ForeignLanguage2EnglishMap, bool);
  virtual ~Household()
  {
  }

private:
  void AL_new();
  int getEstimate(int); // removes the need for hard coding possible word lengths to keep track of estimates
  void AL_attempt(std::string);
  void processEvent(IOEvent& e);
  int getMax(int, int, int);
  std::string householdNameToSound(const std::string& animalName);//Accepts the name of the animal in any language, and returns the corresponding english sound

private:
  enum observation
  {
    right, wrong
  };

  IOEventParser& iep; //So flushGlyph() can be called
  SoundsUtil* su;

  const Voice everyday_s;

  const std::vector<std::string> alphabet;
  const ForeignLanguage2EnglishMap short_sounds; //changed from short_animals
  const ForeignLanguage2EnglishMap med_sounds;
  const ForeignLanguage2EnglishMap long_sounds;

  std::vector<KnowledgeTracer> letter_skill;
  bool nomirror;
  bool firsttime;
  int turncount;
  std::string word; //also name of the object
  std::string target_letter;
  std::string last_word;
  int word_pos;
  int word_length;
  KnowledgeTracer LS_length_skill[8];
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishHousehold : public Household
{
public:
  explicit EnglishHousehold(IOEventParser&);
  ~EnglishHousehold()
  {
  }
private:

  const std::vector<std::string> createAlphabet() const;
  const ForeignLanguage2EnglishMap createShortHouseholdWords() const;
  const ForeignLanguage2EnglishMap createMedHouseholdWords() const;
  const ForeignLanguage2EnglishMap createLongHouseholdWords() const;
};

class English2Household : public Household
{
public:
  explicit English2Household(IOEventParser&);
  ~English2Household()
  {
  }
private:

  const std::vector<std::string> createAlphabet() const;
  const ForeignLanguage2EnglishMap createShortHouseholdWords() const;
  const ForeignLanguage2EnglishMap createMedHouseholdWords() const;
  const ForeignLanguage2EnglishMap createLongHouseholdWords() const;
};

#endif /* HOUSEHOLD_H_ */