package org.techbridgeworld.bwt.teacher;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnHoverListener;
import android.widget.Button;

/**
 * SoundActivity is the third activity in this application. In this activity,
 * the teacher is prompted to select a sound.
 * 
 * @author neharathi
 */
public class SoundActivity extends Activity {

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

	// The directory containing the audio files
	private String dir;

	// The sound options
	private String[] options;

	// The current list of sounds being displayed
	private int currentList = 0;

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
		application.prompt = getResources().getString(R.string.sound_prompt);
		application.help = getResources().getString(R.string.sound_help);

		// Speak the prompt text aloud
		application.speakOut(application.prompt);

		// Initialize the media player
		player = new MediaPlayer();

		// Get the directory containing the audio files
		dir = getApplicationContext().getFilesDir().getPath().toString();

		/*
		 * Initialize options based on the value of applications' category
		 * variable.
		 */
		switch (application.category) {
		// Numbers (Learn Dots, Learn Letters, Animal Game)
		case 0:
		case 2:
		case 5:
			options = new String[6];
			options[0] = getResources().getString(R.string.one);
			options[1] = getResources().getString(R.string.two);
			options[2] = getResources().getString(R.string.three);
			options[3] = getResources().getString(R.string.four);
			options[4] = getResources().getString(R.string.five);
			options[5] = getResources().getString(R.string.six);
			break;
		// Numbers (Hangman)
		case 8:
			options = new String[9];
			options[0] = getResources().getString(R.string.one);
			options[1] = getResources().getString(R.string.two);
			options[2] = getResources().getString(R.string.three);
			options[3] = getResources().getString(R.string.four);
			options[4] = getResources().getString(R.string.five);
			options[5] = getResources().getString(R.string.next_items);

			options[6] = getResources().getString(R.string.six);
			options[7] = getResources().getString(R.string.seven);
			options[8] = getResources().getString(R.string.eight);
			break;
		// Letters (Learn Letters, Animal Game, Hangman)
		case 3:
		case 6:
		case 9:
			options = new String[30];
			options[0] = getResources().getString(R.string.a);
			options[1] = getResources().getString(R.string.b);
			options[2] = getResources().getString(R.string.c);
			options[3] = getResources().getString(R.string.d);
			options[4] = getResources().getString(R.string.e);
			options[5] = getResources().getString(R.string.next_items);

			options[6] = getResources().getString(R.string.f);
			options[7] = getResources().getString(R.string.g);
			options[8] = getResources().getString(R.string.h);
			options[9] = getResources().getString(R.string.i);
			options[10] = getResources().getString(R.string.j);
			options[11] = getResources().getString(R.string.next_items);

			options[12] = getResources().getString(R.string.k);
			options[13] = getResources().getString(R.string.l);
			options[14] = getResources().getString(R.string.m);
			options[15] = getResources().getString(R.string.n);
			options[16] = getResources().getString(R.string.o);
			options[17] = getResources().getString(R.string.next_items);

			options[18] = getResources().getString(R.string.p);
			options[19] = getResources().getString(R.string.q);
			options[20] = getResources().getString(R.string.r);
			options[21] = getResources().getString(R.string.s);
			options[22] = getResources().getString(R.string.t);
			options[23] = getResources().getString(R.string.next_items);

			options[24] = getResources().getString(R.string.u);
			options[25] = getResources().getString(R.string.v);
			options[26] = getResources().getString(R.string.w);
			options[27] = getResources().getString(R.string.x);
			options[28] = getResources().getString(R.string.y);
			options[29] = getResources().getString(R.string.z);
			break;
		// Phrases (Learn Dots)
		case 1:
			options = new String[3];
			options[0] = getResources().getString(R.string.find_dot);
			options[1] = getResources().getString(R.string.good);
			options[2] = getResources().getString(R.string.no);
			break;
		// Phrases (Learn Letters)
		case 4:
			options = new String[5];
			options[0] = getResources().getString(R.string.good);
			options[1] = getResources().getString(R.string.no);
			options[2] = getResources().getString(R.string.please_press);
			options[3] = getResources().getString(R.string.please_write);
			options[4] = getResources().getString(R.string.to_write_the_letter);
			break;
		// Phrases (Animal Game)
		case 7:
			options = new String[9];
			options[0] = getResources().getString(R.string.good);
			options[1] = getResources().getString(R.string.invalid_input);
			options[2] = getResources().getString(R.string.no);
			options[3] = getResources().getString(R.string.please_write);
			options[4] = getResources().getString(
					R.string.please_write_the_name);
			options[5] = getResources().getString(R.string.next_items);

			options[6] = getResources().getString(R.string.press);
			options[7] = getResources().getString(
					R.string.the_correct_answer_was);
			options[8] = getResources().getString(R.string.to_write_the_letter);
			break;
		// Phrases (Hangman)
		case 10:
			options = new String[15];
			options[0] = getResources().getString(R.string.but_you_have);
			options[1] = getResources().getString(R.string.dash);
			options[2] = getResources().getString(R.string.good);
			options[3] = getResources().getString(R.string.guess_a_letter);
			options[4] = getResources().getString(R.string.invalid_input);
			options[5] = getResources().getString(R.string.next_items);

			options[6] = getResources().getString(R.string.letters);
			options[7] = getResources().getString(R.string.mistake);
			options[8] = getResources().getString(R.string.mistakes);
			options[9] = getResources().getString(R.string.no);
			options[10] = getResources().getString(R.string.so_far);
			options[11] = getResources().getString(R.string.next_items);

			options[12] = getResources().getString(R.string.the_new_word);
			options[13] = getResources().getString(R.string.youve_already);
			options[14] = getResources().getString(R.string.youve_made);
			break;
		// Words (Hangman)
		case 11:
			/*
			 * If hangmanWords is empty, alert the user and tell to ensure that
			 * the server is running and an Internet connection is established
			 * before going to CategoryActivity.
			 */
			if (application.hangmanWords.size() == 0) {
				AlertDialog.Builder builder = new AlertDialog.Builder(
						SoundActivity.this);
				builder.setMessage(R.string.no_words).setPositiveButton(
						R.string.ok, new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int id) {
								Intent intent = new Intent(SoundActivity.this,
										CategoryActivity.class);
								startActivity(intent);
							}
						});
				AlertDialog dialog = builder.create();
				dialog.show();
			}

			// Populate options using applications' hangmanWords object
			options = new String[application.hangmanWords.size()
					+ application.hangmanWords.size() / 6];
			if (application.hangmanWords != null) {
				int optCount = 0;
				for (int i = 0; i < application.hangmanWords.size(); i++) {
					if ((optCount + 1) % 6 == 0) {
						options[optCount] = getResources().getString(
								R.string.next_items);
						i--;
					} else {
						options[optCount] = application.hangmanWords.get(i);
					}
					optCount++;
				}
			}
			break;
		default:
			options = null;
		}

		// Initialize buttons such that it has the maximum length of 6
		buttons = new Button[6];
		buttons[0] = (Button) findViewById(R.id.one);
		buttons[1] = (Button) findViewById(R.id.two);
		buttons[2] = (Button) findViewById(R.id.three);
		buttons[3] = (Button) findViewById(R.id.four);
		buttons[4] = (Button) findViewById(R.id.five);
		buttons[5] = (Button) findViewById(R.id.six);

		// Display the current list of sounds
		displayList();
	}

	/**
	 * Displays the current list of sounds.
	 */
	private void displayList() {
		// For each button, display it if it has a corresponding option
		for (int i = 0; i < buttons.length; i++) {
			final int j = 6 * currentList + i;
			/*
			 * If this button has a corresponding option, set its text and
			 * content description to that option and visibility to true.
			 */
			if (j < options.length) {
				buttons[i].setText(options[j]);
				buttons[i].setContentDescription(options[j]);
				buttons[i].setVisibility(View.VISIBLE);
				/*
				 * When a button is clicked, set application's sound variable
				 * accordingly and start RecordActivity.
				 */
				buttons[i].setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						application.sound = options[j];
						Intent intent = new Intent(SoundActivity.this,
								RecordActivity.class);
						startActivity(intent);
					}
				});
				/*
				 * When a button is hovered, play the current recording for the
				 * corresponding sound.
				 */
				buttons[i].setOnHoverListener(new OnHoverListener() {
					@Override
					public boolean onHover(View v, MotionEvent event) {
						if (!player.isPlaying()) {
							FileInputStream fis;
							String filename = options[j].replaceAll(" ", "_");
							try {
								fis = new FileInputStream(dir + "/" + filename
										+ ".m4a");
								player.reset();
								player.setDataSource(fis.getFD());
								fis.close();
								player.prepare();
								player.start();
							} catch (FileNotFoundException e) {
								if (!tts.isSpeaking())
									application.speakOut(options[j]);
							} catch (IllegalArgumentException e) {
								e.printStackTrace();
							} catch (IllegalStateException e) {
								e.printStackTrace();
							} catch (IOException e) {
								e.printStackTrace();
							}
						}
						return true;
					}
				});
			}
			/*
			 * If this button does not have a corresponding option, set its
			 * visibility to false.
			 */
			else
				buttons[i].setVisibility(View.INVISIBLE);
		}

		/*
		 * When a button that says "next items" is clicked, the next list of
		 * sounds will be displayed.
		 */
		if (buttons[5].getText() == getResources().getString(
				R.string.next_items)) {
			buttons[5].setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					currentList++;
					displayList();
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

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		/*
		 * If the user presses back, go to the previous list if applicable.
		 * Otherwise, go to CategoryActivity.
		 */
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			if (currentList > 0) {
				currentList--;
				displayList();
				return true;
			} else {
				Intent intent = new Intent(SoundActivity.this,
						CategoryActivity.class);
				startActivity(intent);
			}
		}
		return super.onKeyDown(keyCode, event);
	}

}