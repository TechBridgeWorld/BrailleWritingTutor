/*
 * animal.h
 *
 *  Created on: Dec 8, 2008
 *      Author: Students
 *      Multi-lingual enhancements: Imran
 */

#ifndef ANIMAL_H_
#define ANIMAL_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/KnowledgeTracer.h"
#include "common/language_utils.h"
//maps the actual names of the animals in a foreign (ie,non-native) language to their english names
  typedef std::map<std::string,std::string> ForeignLanguage2EnglishMap;
  
class Animal : public IBTApp
{
public:
  explicit Animal(IOEventParser&, const std::string&, SoundsUtil*, const std::vector<std::string>, const ForeignLanguage2EnglishMap, const ForeignLanguage2EnglishMap, const ForeignLanguage2EnglishMap, bool);
  virtual ~Animal()
  {
  }

private:
  void AL_new();
  void AL_attempt(std::string);
  void processEvent(IOEvent& e);
  std::string animalNameToSound(const std::string& animalName);//Accepts the name of the animal in any language, and returns the corresponding english sound

private:
  enum observation
  {
    right, wrong
  };
  const Voice animal_s;
  IOEventParser& iep; //So flushGlyph() can be called
  SoundsUtil* su;


  const std::vector<std::string> alphabet;
  const ForeignLanguage2EnglishMap short_animals;
  const ForeignLanguage2EnglishMap med_animals;
  const ForeignLanguage2EnglishMap long_animals;

  std::vector<KnowledgeTracer> letter_skill;
  bool nomirror;
  bool firsttime;
  int turncount;
  std::string word; //also name of the animal
  std::string target_letter;
  int word_pos;
  int word_length;
  KnowledgeTracer LS_length_skill[8];
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishAnimal : public Animal
{
public:
  explicit EnglishAnimal(IOEventParser&);
  ~EnglishAnimal()
  {
  }
private:

  const std::vector<std::string> createAlphabet() const;
  const ForeignLanguage2EnglishMap createShortAnimalWords() const;
  const ForeignLanguage2EnglishMap createMedAnimalWords() const;
  const ForeignLanguage2EnglishMap createLongAnimalWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicAnimal : public Animal
{
public:
  explicit ArabicAnimal(IOEventParser&);
  ~ArabicAnimal()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const ForeignLanguage2EnglishMap createShortAnimalWords() const;
  const ForeignLanguage2EnglishMap createMedAnimalWords() const;
  const ForeignLanguage2EnglishMap createLongAnimalWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchAnimal : public Animal
{
public:
  explicit FrenchAnimal(IOEventParser&);
  ~FrenchAnimal()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const ForeignLanguage2EnglishMap createShortAnimalWords() const;
  const ForeignLanguage2EnglishMap createMedAnimalWords() const;
  const ForeignLanguage2EnglishMap createLongAnimalWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliAnimal : public Animal
{
public:
  explicit KiswahiliAnimal(IOEventParser&);
  ~KiswahiliAnimal()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const ForeignLanguage2EnglishMap createShortAnimalWords() const;
  const ForeignLanguage2EnglishMap createMedAnimalWords() const;
  const ForeignLanguage2EnglishMap createLongAnimalWords() const;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2Animal : public Animal
{
public:
  explicit English2Animal(IOEventParser&);
  ~English2Animal()
  {
  }
private:

  const std::vector<std::string> createAlphabet() const;
  const ForeignLanguage2EnglishMap createShortAnimalWords() const;
  const ForeignLanguage2EnglishMap createMedAnimalWords() const;
  const ForeignLanguage2EnglishMap createLongAnimalWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2Animal : public Animal
{
public:
  explicit Arabic2Animal(IOEventParser&);
  ~Arabic2Animal()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const ForeignLanguage2EnglishMap createShortAnimalWords() const;
  const ForeignLanguage2EnglishMap createMedAnimalWords() const;
  const ForeignLanguage2EnglishMap createLongAnimalWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif /* ANIMAL_H_ */
