/*
 * language_utils.h
 *
 *  Created on: Dec 18, 2008
 *      Author: imran
 */

#ifndef LANGUAGE_UTILS_H_
#define LANGUAGE_UTILS_H_

#include "Voice/Voice.h"

class SoundsUtil
{
public:
  explicit SoundsUtil()
  {
  }
  virtual ~SoundsUtil()
  {
  }
public:
  typedef unsigned char DotSequence;
  virtual void sayNumber(const Voice&, int, bool) const = 0;
  virtual void sayLetter(const Voice&, const std::string&) const  = 0;
  virtual void sayLetterSequence(const Voice&, const std::string&) const = 0;
  virtual void sayDotSequence(const Voice&, DotSequence d) const = 0;
  virtual void saySound(const Voice&, const std::string&) const = 0;
};

class EnglishSoundsUtil : public SoundsUtil
{
public:
  explicit EnglishSoundsUtil()
  {
  }
  virtual ~EnglishSoundsUtil()
  {
  }
  void sayNumber(const Voice&, int, bool) const ;
  void sayLetter(const Voice&, const std::string&) const ;
  void sayLetterSequence(const Voice&, const std::string&) const ;
  void sayDotSequence(const Voice&, DotSequence d) const;
  void saySound(const Voice&, const std::string&) const ;
};

class ArabicSoundsUtil : public SoundsUtil
{
public:
  explicit ArabicSoundsUtil()
  {
  }
  virtual ~ArabicSoundsUtil()
  {
  }
  void sayNumber(const Voice&, int, bool) const ;
  void sayLetter(const Voice&, const std::string&) const ;
  void sayLetterSequence(const Voice&, const std::string&) const ;
  void sayDotSequence(const Voice&, DotSequence d) const ;
  void saySound(const Voice&, const std::string&) const ;
private:
  typedef std::map<std::string, std::string> ArabicLettersToSoundFilesMap;
  static const ArabicSoundsUtil::ArabicLettersToSoundFilesMap arabic_letter_map;
};

class FrenchSoundsUtil : public SoundsUtil
{
public:
  explicit FrenchSoundsUtil()
  {
  }
  virtual ~FrenchSoundsUtil()
  {
  }
  void sayNumber(const Voice&, int, bool) const ;
  void sayLetter(const Voice&, const std::string&) const ;
  void sayLetterSequence(const Voice&, const std::string&) const ;
  void sayDotSequence(const Voice&, DotSequence d) const ;
  void saySound(const Voice&, const std::string&) const ;
private:
  typedef std::map<std::string, std::string> FrenchLettersToSoundFilesMap;
  static const FrenchSoundsUtil::FrenchLettersToSoundFilesMap french_letter_map;
};

class KiswahiliSoundsUtil : public SoundsUtil
{
public:
  explicit KiswahiliSoundsUtil()
  {
  }
  virtual ~KiswahiliSoundsUtil()
  {
  }
  void sayNumber(const Voice&, int, bool) const ;
  void sayLetter(const Voice&, const std::string&) const ;
  void sayLetterSequence(const Voice&, const std::string&) const ;
  void sayDotSequence(const Voice&, DotSequence d) const ;
  void saySound(const Voice&, const std::string&) const ;
private:
  typedef std::map<std::string, std::string> KiswahiliLettersToSoundFilesMap;
  static const KiswahiliSoundsUtil::KiswahiliLettersToSoundFilesMap kiswahili_letter_map;
};


class Arabic2SoundsUtil : public SoundsUtil
{
public:
  explicit Arabic2SoundsUtil()
  {
  }
  virtual ~Arabic2SoundsUtil()
  {
  }
  void sayNumber(const Voice&, int, bool) const ;
  void sayLetter(const Voice&, const std::string&) const ;
  void sayLetterSequence(const Voice&, const std::string&) const ;
  void sayDotSequence(const Voice&, DotSequence d) const ;
  void saySound(const Voice&, const std::string&) const ;
private:
  typedef std::map<std::string, std::string> Arabic2LettersToSoundFilesMap;
  static const Arabic2SoundsUtil::Arabic2LettersToSoundFilesMap arabic2_letter_map;
};

class English2SoundsUtil : public SoundsUtil
{
public:
  explicit English2SoundsUtil()
  {
  }
  virtual ~English2SoundsUtil()
  {
  }
  void sayNumber(const Voice&, int, bool) const ;
  void sayLetter(const Voice&, const std::string&) const ;
  void sayLetterSequence(const Voice&, const std::string&) const ;
  void sayDotSequence(const Voice&, DotSequence d) const;
  void saySound(const Voice&, const std::string&) const ;
};

#endif /* LANGUAGE_UTILS_H_ */
