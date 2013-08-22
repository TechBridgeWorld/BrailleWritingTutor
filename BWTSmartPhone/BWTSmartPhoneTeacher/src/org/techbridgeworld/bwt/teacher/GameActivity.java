package org.techbridgeworld.bwt.teacher;

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;

import org.techbridgeworld.bwt.teacher.R.raw;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources.NotFoundException;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

/**
 * GameActivity is the first activity in this application. In this activity, the
 * teacher is prompted to select a game.
 * 
 * @author neharathi
 */
public class GameActivity extends Activity {

	// The global application
	private MyApplication application;

	// Speaks text aloud
	private TextToSpeech tts;

	// Detects the phone shaking
	private SensorManager manager;
	private ShakeEventListener listener;

	// The UI buttons
	private Button[] buttons;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.list);

		// Get the global application
		application = ((MyApplication) getApplicationContext());

		// Retrieve the global objects from application
		tts = application.myTTS;
		manager = application.myManager;
		listener = application.myListener;

		// Set the prompt and help text
		application.prompt = getResources().getString(R.string.game_prompt);
		application.help = getResources().getString(R.string.game_help);

		// Speak the prompt text aloud
		application.speakOut(application.prompt);

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
			 * accordingly and start CategoryActivity.
			 */
			buttons[i].setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					application.game = j;
					Intent intent = new Intent(GameActivity.this,
							CategoryActivity.class);
					startActivity(intent);
				}
			});
		}

		/*
		 * If this is the first run of GameActivity, set the prompt text to be a
		 * more detailed version, store the audio files on internal storage, and
		 * note that the first run of GameActivity has occurred.
		 */
		SharedPreferences prefs = getSharedPreferences("BWT", MODE_PRIVATE);
		if (prefs.getBoolean("firstRun", true)) {
			// Set the prompt text to be more detailed version
			application.prompt = getResources().getString(
					R.string.game_prompt_first);
			// Store the audio files on internal storage
			Class<raw> raw = R.raw.class;
			Field[] fields = raw.getFields();
			for (Field field : fields) {
				try {
					InputStream is = getResources().openRawResource(
							field.getInt(null));
					ByteArrayOutputStream baos = new ByteArrayOutputStream();

					int size = 0;
					byte[] buffer = new byte[1024];
					while ((size = is.read(buffer, 0, 1024)) >= 0) {
						baos.write(buffer, 0, size);
					}
					is.close();
					buffer = baos.toByteArray();

					FileOutputStream fos = openFileOutput(field.getName()
							+ ".m4a", MODE_PRIVATE);
					fos.write(buffer);
					fos.close();
				} catch (NotFoundException e) {
					e.printStackTrace();
				} catch (IllegalArgumentException e) {
					e.printStackTrace();
				} catch (IllegalAccessException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			// Note that the first run of GameActivity has occurred
			SharedPreferences.Editor editor = prefs.edit();
			editor.putBoolean("firstRun", false);
			editor.commit();
		}
	}

	@Override
	protected void onResume() {
		// Register listener to manager with the appropriate arguments
		manager.registerListener(listener,
				manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
				SensorManager.SENSOR_DELAY_UI);
		super.onResume();
	}

	@Override
	protected void onPause() {
		// Unregister listener from manager and stop text to speech
		manager.unregisterListener(listener);
		tts.stop();
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
