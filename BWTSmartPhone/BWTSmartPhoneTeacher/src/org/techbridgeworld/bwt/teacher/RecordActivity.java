package org.techbridgeworld.bwt.teacher;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

/**
 * RecordActivity is the last activity in this application. In this activity,
 * the teacher is instructed to record the sound.
 * 
 * @author neharathi
 */
public class RecordActivity extends Activity {

	// The global application
	private MyApplication application;

	// Speaks text aloud
	private TextToSpeech tts;

	// Detects the phone shaking
	private SensorManager manager;
	private ShakeEventListener listener;

	// The UI buttons
	private Button[] buttons;

	// Controls playing of audio files
	private MediaPlayer player;

	// Controls recording of audio files
	private MediaRecorder recorder;

	// Represents whether the teacher is recording a sound
	private boolean isRecording;

	// Represents whether the teacher has recorded a sound
	private boolean hasRecorded;

	// The application context
	private Context context;

	// The directory containing the audio files
	private String dir;

	// The filename of the audio file
	private String filename;

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
		application.prompt = getResources().getString(R.string.record_prompt);
		application.help = getResources().getString(R.string.record_help);

		// Speak the prompt text aloud
		application.speakOut(application.prompt);

		// Initialize the media player and media recorder
		player = new MediaPlayer();
		recorder = new MediaRecorder();

		// Initialize isRecording and hasRecorded to false
		isRecording = false;
		hasRecorded = false;

		// Get the application context
		context = getApplicationContext();

		// Get the directory containing the audio files
		dir = context.getFilesDir().getPath().toString();

		// Create an array containing the recording options
		String options[] = new String[3];
		options[0] = getResources().getString(R.string.play);
		options[1] = getResources().getString(R.string.save);
		options[2] = getResources().getString(R.string.cancel);

		// Initialize buttons such that it has the same length as options
		buttons = new Button[3];

		/*
		 * "play" When this button is clicked, play the appropriate sound
		 */
		buttons[0] = (Button) findViewById(R.id.one);
		buttons[0].setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if (!player.isPlaying()) {
					FileInputStream fis;
					filename = application.sound.replaceAll(" ", "_");
					try {
						/*
						 * If the teacher has not recorded a sound, play the
						 * current recording.
						 */
						if (!hasRecorded)
							fis = new FileInputStream(dir + "/" + filename
									+ ".m4a");
						/*
						 * If the teacher has recorded a sound, play her new
						 * recording.
						 */
						else
							fis = new FileInputStream(dir + "/" + filename
									+ "_temp.m4a");
						player.reset();
						player.setDataSource(fis.getFD());
						fis.close();
						player.prepare();
						player.start();
					}
					/*
					 * If there is no recording, speak the sound aloud using
					 * text to speech.
					 */
					catch (FileNotFoundException e) {
						application.speakOut(application.sound);
					} catch (IllegalArgumentException e) {
						e.printStackTrace();
					} catch (IllegalStateException e) {
						e.printStackTrace();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		});

		/*
		 * "save" When this button is clicked, save the teachers' new recording
		 * and go back to SoundActivity.
		 */
		buttons[1] = (Button) findViewById(R.id.two);
		buttons[1].setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				context.deleteFile(filename + ".m4a");
				File oldFile = context
						.getFileStreamPath(filename + "_temp.m4a");
				File newFile = context.getFileStreamPath(filename + ".m4a");
				oldFile.renameTo(newFile);

				Intent intent = new Intent(RecordActivity.this,
						SoundActivity.class);
				startActivity(intent);
			}
		});

		/*
		 * "cancel" When this button is clicked, cancel the teachers' new
		 * recording and go back to SoundActivity.
		 */
		buttons[2] = (Button) findViewById(R.id.three);
		buttons[2].setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				context.deleteFile(filename + "_temp.m4a");

				Intent intent = new Intent(RecordActivity.this,
						SoundActivity.class);
				startActivity(intent);
			}
		});

		/*
		 * For each option, set the corresponding buttons' text and content
		 * description to that option and visibility to true.
		 */
		for (int i = 0; i < options.length; i++) {
			buttons[i].setText(options[i]);
			buttons[i].setContentDescription(options[i]);
			buttons[i].setVisibility(View.VISIBLE);
		}
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		int action = event.getAction();
		if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP) {
			// If the user holds the volume down button, start recording
			if (action == KeyEvent.ACTION_DOWN && isRecording == false) {
				tts.stop();
				filename = application.sound.replaceAll(" ", "_");
				recorder.setAudioSource(MediaRecorder.AudioSource.MIC);
				recorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
				recorder.setAudioEncoder(MediaRecorder.AudioEncoder.DEFAULT);
				recorder.setOutputFile(dir + "/" + filename + "_temp.m4a");
				try {
					recorder.prepare();
				} catch (IllegalStateException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
				recorder.start();
				if (!hasRecorded)
					hasRecorded = true;
				isRecording = true;
			}
			// If the user releases the volume up button, stop recording
			else if (action == KeyEvent.ACTION_UP && isRecording == true) {
				recorder.reset();
				isRecording = false;
			}
			return true;
		}
		return super.dispatchKeyEvent(event);
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
		// If the user presses back, treat it as if they pressed "cancel"
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			context.deleteFile(filename + "_temp.m4a");
			Intent intent = new Intent(RecordActivity.this, SoundActivity.class);
			startActivity(intent);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
}