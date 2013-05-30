/*
 * BTApp.h
 *
 *  Created on: Dec 1, 2008
 *      Author: imran
 */

#ifndef BTAPP_H_
#define BTAPP_H_
#include "utilities.h"
#include "Voice/Voice.h"


class IBTApp
{
public:
  //explicit IBTApp(); //every subclass calls this constructor implicitly (ie, silently)
  explicit IBTApp(IOEventParser&, const std::string&);
  virtual ~IBTApp(); //important that this is virtual. Otherwise the destructor of the derived class will not be called. For example, the destructor of the Dominos app is crucial because it stop()s the game, hence preventing previously spawned TimerTask threads from interfering with another BT app.
  const Voice& getTeacherVoice() const;
  const Voice& getStudentVoice() const;
  static const Charset& getCurrentCharset();
public:
  virtual void processEvent(IOEvent& event) = 0;
private:
  void loadLanguageCharset(const std::string&);
  void loadDefaultCharset();

private:
  IOEventParser& iep;
  Voice teacher_voice;
  Voice student_voice;

  /*
   * Incase you are wondering why currentCharset member is static(instead of a normal member). First note that
   * the BT library's IOEventParser.setCharset() method DOES NOT make it's own copy of the Charset, it refers
   * to the one you specify it IOEventParser::setCharset(currentCharset)...so to be on the "safe" side
   * currentCharset is static
   */
  static Charset currentCharset;
};

#endif /* BTAPP_H_ */
