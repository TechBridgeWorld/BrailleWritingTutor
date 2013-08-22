package org.techbridgeworld.bwt.teacher;

import android.app.Activity;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

/**
 * CategoryActivity is the second activity in this application. In this
 * activity, the teacher is prompted to select a category.
 * 
 * @author neharathi
 */
public class CategoryActivity extends Activity {

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
		application.prompt = getResources().getString(R.string.category_prompt);
		application.help = getResources().getString(R.string.category_help);

		// Speak the prompt text aloud
		application.speakOut(application.prompt);

		/*
		 * Create an array containing the category options based on the value of
		 * applications' game variable.
		 */
		String options[] = null;
		switch (application.game) {
		// Learn Dots
		case 0:
			options = new String[2];
			options[0] = getResources().getString(R.string.numbers);
			options[1] = getResources().getString(R.string.instructions);
			break;
		// Learn Letters, Animal Game
		case 1:
		case 2:
			options = new String[3];
			options[0] = getResources().getString(R.string.numbers);
			options[1] = getResources().getString(R.string.letters);
			options[2] = getResources().getString(R.string.instructions);
			break;
		// Hangman
		case 3:
			options = new String[4];
			options[0] = getResources().getString(R.string.numbers);
			options[1] = getResources().getString(R.string.letters);
			options[2] = getResources().getString(R.string.instructions);
			options[3] = getResources().getString(R.string.words);
			break;
		default:
		}

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
			 * When a button is clicked, set application's category variable
			 * accordingly and start OptionsActivity.
			 */
			buttons[i].setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					application.category = application.game * 3 + j - 1;
					application.category += application.game == 0 ? 1 : 0;
					Intent intent = new Intent(CategoryActivity.this,
							SoundActivity.class);
					startActivity(intent);
				}
			});
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

	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// If the user presses back, go to GameActivity
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			Intent intent = new Intent(CategoryActivity.this,
					GameActivity.class);
			startActivity(intent);
		}
		return super.onKeyDown(keyCode, event);
	}
}