package org.techbridgeworld.bwt.student;

import java.util.Date;
import java.util.Random;
import javaEventing.interfaces.Event;
import javaEventing.interfaces.GenericEventListener;

import org.techbridgeworld.bwt.api.BWT;
import org.techbridgeworld.bwt.api.events.SubmitEvent;
import org.techbridgeworld.bwt.api.libs.Braille;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.util.Log;
import android.view.KeyEvent;

/**
 * AnimalGame replicates the functionality of Animal Game on the BWT. This game
 * prompts students to spell the name of an animal by providing the sound the
 * animal makes. If the user inputs a wrong letter three times in a row, the
 * stage will change to provide more guidance to the user.
 * 
 * Stages: 1. Provide the sound of the animal 2. Spell the name of the animal 3.
 * Give the dot numbers for the required letter
 * 
 * @author jessicalo
 * 
 */
public class AnimalGame extends Activity {

	// The global application
	private MyApplication application;

	// Speaks text aloud
	private TextToSpeech tts;

	// The BWT
	private final BWT bwt = new BWT(this, AnimalGame.this);

	// Contains mappings of dots in a Braille cell to a character
	private static final Braille braille = new Braille();

	// Randomly generates a new animal to test
	private Random generator = new Random(new Date().getTime());

	// List of animals to be tested
	private final String[] animals = { "bee", "camel", "cat", "cow", "dog",
			"horse", "pig", "rooster", "sheep", "zebra" };

	// Represents the stage a user is in
	private int stage;

	// Helps define the three stages
	private final int ANIM_SOUND_STAGE = 0;
	private final int SPELL_ANIM_STAGE = 1;
	private final int GIVE_DOTS_STAGE = 2;

	// Maximum number of mistakes before stage changes
	private final int MAX_MISTAKES = 3;

	// Number of mistakes in a row
	private int mistakes;

	// The current animal being tested
	private String currAnimal = "";

	// The current letter index of the current animal
	private int currLetterInd;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.list);

		// Get the global application and global text to speech
		application = ((MyApplication) getApplicationContext());
		tts = application.myTTS;

		// Initialize currentFile and filenames
		application.currentFile = 0;
		application.filenames.clear();

		// Initialize the BWT connection.
		bwt.init();

		// Start the BWT
		bwt.start();

		// Start tracking the state of the BWT
		bwt.initializeEventListeners();
		bwt.startTracking();
		runGame();
	}

	@Override
	public void onPause() {
		// Clear the audio queue and stop the BWT
		application.clearAudio();
		bwt.stopTracking();
		bwt.removeEventListeners();
		bwt.stop();
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

	/**
	 * @return the name of the audio file holding the current animal's sound
	 */
	private String getCurrSound() {
		return currAnimal + "_sound";
	}

	/**
	 * Randomly generates a new animal to test and resets the game variables.
	 */
	private void regenerate() {
		currAnimal = animals[generator.nextInt(animals.length)];
		currLetterInd = 0;
		mistakes = 0;
		stage = ANIM_SOUND_STAGE;
	}

	/**
	 * Provides the first instructions and creates listeners for the BWT board.
	 */
	private void runGame() {
		// Generate the first animal
		regenerate();

		// Provide the first instructions
		gameInstruction();
		application.playAudio();

		// Create listeners for the BWT board
		createListeners();
	}

	/**
	 * Provides instruction depending on the stage.
	 * 
	 * ANIM_SOUND:
	 * "Please write the name of the animal that makes this sound: [sound]."
	 * SPELL_ANIM: "Please write [letter], [letter], ..." GIVE_DOTS:
	 * "To write the letter [letter], please press [number], [number], ..."
	 */
	private void gameInstruction() {
		// If the user is in ANIM_SOUND_STAGE, provide the sound of the animal
		if (stage == ANIM_SOUND_STAGE) {
			application.queueAudio(R.string.please_write_the_name);
			application.queueAudio(getCurrSound());

		}
		// If the user is in SPELL_ANIM_STAGE, spell the name of the animal
		else if (stage == SPELL_ANIM_STAGE) {
			application.queueAudio(R.string.please_write);
			spellCurrAnimal();
		}
		/*
		 * If the user is in GIVE_DOTS_STSAGE, give the dots for the last
		 * incorrect letter.
		 */
		else if (stage == GIVE_DOTS_STAGE) {
			char currLetter = currAnimal.charAt(currLetterInd);
			int btns = braille.get(currLetter);

			application.queueAudio(R.string.to_write_the_letter);
			application.queueAudio(((Character) currLetter).toString());
			application.queueAudio(R.string.please_press);
			for (int i = 0; i < 6; i++) {
				if ((btns & (1 << i)) > 0) {
					String num = ((Integer) (i + 1)).toString();
					application.queueAudio(num);
				}
			}
		}
	}

	/**
	 * Spell the name of the current animal
	 */
	private void spellCurrAnimal() {
		application.queueAudio(currAnimal);

		for (int i = 0; i < currAnimal.length(); i++) {
			Character letter = currAnimal.charAt(i);
			application.queueAudio(letter.toString());
		}
	}

	/**
	 * Processes every glyph written and queues/plays audio as necessary.
	 */
	private void createListeners() {
		// Compares the expected word to the user input
		GenericEventListener AnimalListener = new GenericEventListener() {
			@Override
			public void eventTriggered(Object arg0, Event arg1) {
				bwt.defaultSubmitHandler(arg0, arg1);
				SubmitEvent e = (SubmitEvent) arg1;
				Log.d("Animal Game", "Triggered Submit Event");

				// Get the finished glyph and reset the bits at that cell to 0
				int cellInd = e.getCellInd();
				char glyphAtCell = bwt.getGlyphAtCell(cellInd);
				bwt.setBitsAtCell(cellInd, 0);

				/*
				 * If the glyph is invalid, tell the user so. Otherwise, speak
				 * the character aloud.
				 */
				if (glyphAtCell == '-') {
					application.queueAudio(R.string.invalid_input);
				} else {
					String chStr = ((Character) glyphAtCell).toString();
					application.queueAudio(chStr);
				}

				char expectedChar = currAnimal.charAt(currLetterInd);

				/*
				 * Compare the glyph to the expected character and handle the
				 * result accordingly.
				 */
				if (glyphAtCell != expectedChar)
					wrongCharacterHandler();
				else
					correctCharacterHandler();

				application.playAudio();
			}
		};
		bwt.replaceListener("onSubmitEvent", AnimalListener);
	}

	/**
	 * Handles the glyph not matching the expected character based on the stage
	 * and number of mistakes. Called when the user is wrong.
	 */
	private void wrongCharacterHandler() {
		// Tell the user they are wrong and update mistakes
		application.queueAudio(R.string.no);
		mistakes++;

		// If the user is in GIVE_DOTS, repeat the instructions
		if (stage == GIVE_DOTS_STAGE) {
			gameInstruction();
			return;
		}

		/*
		 * If the user has had three mistakes, increase the stage number.
		 * Otherwise, reset currLetterInd to 0.
		 */
		if (mistakes == MAX_MISTAKES) {
			if (stage == SPELL_ANIM_STAGE) {
				stage = GIVE_DOTS_STAGE;
			} else {
				stage = SPELL_ANIM_STAGE;
				application.queueAudio(R.string.the_correct_answer_was);
				spellCurrAnimal();
				currLetterInd = 0;
			}
			mistakes = 0;
		} else {
			currLetterInd = 0;
		}

		// Provide the appropriate instructions
		gameInstruction();
	}

	/**
	 * Handles the glyph matching the expected character. Called when the user
	 * is correct.
	 */
	private void correctCharacterHandler() {

		/*
		 * If the user is in GIVE_DOTS, upgrade them to SPELL_ANIM and provide
		 * the appropriate instructions.
		 */
		if (stage == GIVE_DOTS_STAGE) {
			application.queueAudio(R.string.good);
			stage = SPELL_ANIM_STAGE;
			mistakes = 0;
			currLetterInd = 0;
			gameInstruction();
		}
		/*
		 * Otherwise, wait until the word is completed.
		 */
		else {
			currLetterInd++;

			/*
			 * When the user is in SPELL_ANIM, the mistakes should not be reset
			 * until the user reaches the end of the word.
			 */
			if (stage != SPELL_ANIM_STAGE
					|| currLetterInd == currAnimal.length()) {
				mistakes = 0;
			}

			/*
			 * If the user reaches the end of the word, randomly generate a new
			 * animal to test and provide the appropriate instructions.
			 */
			if (currLetterInd == currAnimal.length()) {
				application.queueAudio(R.string.good);
				bwt.resetBoard();
				regenerate();
				gameInstruction();
			}
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// If the user presses back, go to GameActivity
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			bwt.removeEventListeners();
			Intent intent = new Intent(AnimalGame.this, GameActivity.class);
			startActivity(intent);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
}