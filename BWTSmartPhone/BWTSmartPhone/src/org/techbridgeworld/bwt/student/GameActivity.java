package org.techbridgeworld.bwt.student;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Process;
import android.speech.tts.TextToSpeech;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

/**
 * GameActivity is the first activity in this application. In this activity, the
 * teacher is prompted to select a game for her student to play.
 * 
 * @author neharathi
 */
public class GameActivity extends Activity {

	// The global application
	private MyApplication application;

	// Speaks text aloud
	private TextToSpeech tts;

	// The UI buttons
	private Button[] buttons;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.list);

		// Get the global application and global text to speech
		application = ((MyApplication) getApplicationContext());
		tts = application.myTTS;

		// Set the prompt and speak it aloud
		application.prompt = getResources().getString(R.string.game_prompt);
		application.speakOut(application.prompt);

		/*
		 * If the teacher app has not been installed, alert the user that it
		 * must be installed and opened before the student app.
		 */
		if (application.context == null) {
			AlertDialog.Builder builder = new AlertDialog.Builder(
					GameActivity.this);
			builder.setMessage(R.string.install_teacher).setPositiveButton(
					R.string.ok, new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int id) {
							finish();
							Process.killProcess(Process.myPid());
						}
					});
			AlertDialog dialog = builder.create();
			dialog.show();
		}
		/*
		 * If the teacher app has been installed but not opened, alert the user
		 * that it must be opened before the student app.
		 */
		else {
			SharedPreferences prefs = application.context.getSharedPreferences(
					"BWT", MODE_PRIVATE);
			if (prefs.getBoolean("firstRun", true)) {
				AlertDialog.Builder builder = new AlertDialog.Builder(
						GameActivity.this);
				builder.setMessage(R.string.open_teacher).setPositiveButton(
						R.string.ok, new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int id) {
								finish();
								Process.killProcess(Process.myPid());
							}
						});
				AlertDialog dialog = builder.create();
				dialog.show();
			}
		}

		// Create an array containing the game options
		String[] options = new String[4];
		options[0] = getResources().getString(R.string.learn_dots);
		options[1] = getResources().getString(R.string.learn_letters);
		options[2] = getResources().getString(R.string.animal_game);
		options[3] = getResources().getString(R.string.hangman);

		// Initialize buttons such that it has the same length as options
		buttons = new Button[4];
		buttons[0] = (Button) findViewById(R.id.one);
		buttons[1] = (Button) findViewById(R.id.two);
		buttons[2] = (Button) findViewById(R.id.three);
		buttons[3] = (Button) findViewById(R.id.four);

		/*
		 * For each option, set the corresponding buttons' text and content
		 * description to that option and visibility to true.
		 */
		for (int i = 0; i < options.length; i++) {
			final int j = i;
			buttons[i].setText(options[i]);
			buttons[i].setContentDescription(options[i]);
			buttons[i].setVisibility(View.VISIBLE);

			/*
			 * When a button is clicked, set application's game variable
			 * accordingly and start specified game activity.
			 */
			buttons[i].setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					Class<?> c = null;
					switch (j) {
					case 0:
						c = LearnDots.class;
						break;
					case 1:
						c = LearnLetters.class;
						break;
					case 2:
						c = AnimalGame.class;
						break;
					case 3:
						c = Hangman.class;
						break;
					default:
						return;
					}

					Intent intent = new Intent(GameActivity.this, c);
					startActivity(intent);
				}
			});
		}
	}

	@Override
	public void onPause() {
		// Clear the audio queue
		application.clearAudio();
		super.onPause();
	}

	@Override
	public void onDestroy() {
		// Stop and shutdown text to speech
		if (tts != null) {
			tts.stop();
			tts.shutdown();
		}
		super.onDestroy();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// If the user presses back, go to the home screen
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			Intent intent = new Intent(Intent.ACTION_MAIN);
			intent.addCategory(Intent.CATEGORY_HOME);
			intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			startActivity(intent);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
}