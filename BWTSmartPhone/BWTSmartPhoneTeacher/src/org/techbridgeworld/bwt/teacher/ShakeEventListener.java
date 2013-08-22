package org.techbridgeworld.bwt.teacher;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;

/**
 * Listener that detects when the phone is shaken.
 * 
 * This code was taken from
 * http://stackoverflow.com/questions/2317428/android-i-want-to-shake-it.
 */
public class ShakeEventListener implements SensorEventListener {

	// Minimum movement force to consider
	private static final int MIN_FORCE = 10;

	// Minimum times that the direction of movement needs to change
	private static final int MIN_DIRECTION_CHANGE = 3;

	// Maximum pause between movements.
	private static final int MAX_PAUSE_BETHWEEN_DIRECTION_CHANGE = 200;

	// Maximum allowed time for shake gesture
	private static final int MAX_TOTAL_DURATION_OF_SHAKE = 400;

	// Time when the gesture started
	private long mFirstDirectionChangeTime = 0;

	// Time when the last movement started
	private long mLastDirectionChangeTime;

	// How many movements are considered so far
	private int mDirectionChangeCount = 0;

	// The last x position
	private float lastX = 0;

	// The last y position
	private float lastY = 0;

	// The last z position
	private float lastZ = 0;

	// OnShakeListener that is called when shake is detected
	private OnShakeListener mShakeListener;

	/**
	 * Interface for the shake gesture.
	 */
	public interface OnShakeListener {

		/**
		 * Called when the shake gesture is detected
		 */
		void onShake();
	}

	public void setOnShakeListener(OnShakeListener listener) {
		mShakeListener = listener;
	}

	@Override
	public void onSensorChanged(SensorEvent se) {
		// Get sensor data
		float x = se.values[0];
		float y = se.values[1];
		float z = se.values[2];

		// Calculate movement
		float totalMovement = Math.abs(x + y + z - lastX - lastY - lastZ);

		if (totalMovement > MIN_FORCE) {
			// Get time
			long now = System.currentTimeMillis();

			// Store first movement time
			if (mFirstDirectionChangeTime == 0) {
				mFirstDirectionChangeTime = now;
				mLastDirectionChangeTime = now;
			}

			// Check if the last movement was not long ago
			long lastChangeWasAgo = now - mLastDirectionChangeTime;
			if (lastChangeWasAgo < MAX_PAUSE_BETHWEEN_DIRECTION_CHANGE) {

				// Store movement data
				mLastDirectionChangeTime = now;
				mDirectionChangeCount++;

				// Store last sensor data
				lastX = x;
				lastY = y;
				lastZ = z;

				// Check how many movements are so far
				if (mDirectionChangeCount >= MIN_DIRECTION_CHANGE) {

					// Check total duration
					long totalDuration = now - mFirstDirectionChangeTime;
					if (totalDuration < MAX_TOTAL_DURATION_OF_SHAKE) {
						mShakeListener.onShake();
						resetShakeParameters();
					}
				}
			} else {
				resetShakeParameters();
			}
		}
	}

	/**
	 * Reset the shake parameters to their default values
	 */
	private void resetShakeParameters() {
		mFirstDirectionChangeTime = 0;
		mDirectionChangeCount = 0;
		mLastDirectionChangeTime = 0;
		lastX = 0;
		lastY = 0;
		lastZ = 0;
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}
}