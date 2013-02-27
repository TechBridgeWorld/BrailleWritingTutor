package edu.cmu.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import edu.cmu.server.JettyServer;

/**
 * A SWING GUI that gives the user the basic control over the embedded jetty
 * server. This should be the only entry point of this program. The web-based
 * emulator and http handlers should not be exposed to the user except from this
 * UI.
 * 
 * @author ziw
 * 
 */
public class ControllerUI extends JFrame {

	//TODO
	//print out error msg.
	//log bytes sent
	
	private static final long serialVersionUID = -583606563274987091L;
	private UIActionHandler handler;

	private static final String WINDOW_LABEL = "BWT Emulator Control";

	// button labels
	public static final String START_BUTTON = "Start Server";
	public static final String STOP_BUTTON = "Stop Server";
	public static final String QUIT_BUTTON = "Quit";
	public static final String LAUNCH = "Launch Emulator";

	// buttons
	private JButton quitButton;
	private JButton startButton;
	private JButton stopButton;
	private JButton launchButton;

	// other components
	private JTextArea statusArea;
	private JTextArea msgArea;
	
	//tooltips
	private static final String START_SERVER_TOOLTIP = "Start the server.";
	private static final String STOP_SERVER_TOOLTIP = "Stop the server.";
	private static final String QUIT_TOOLTIP = "Qui the program.";
	private static final String LAUNCH_TOOLTIP_ON = "Launch the emulator.";
	private static final String LAUNCH_TOOLTIP_OFF = "Can't launch the emulator. Server is offline.";
		
	public ControllerUI() {
		initUI();
		initHandler();
		updateStatusArea(getServerStatus());
	}
	
	private void initHandler(){
		handler = new UIActionHandler();
	}

	private void initUI() {
		JPanel panel = new JPanel();
		getContentPane().add(panel);
		panel.setLayout(null);

		
		initLayout();
		bindActionListener();
		
		panel.add(statusArea);
		panel.add(quitButton);
//		panel.add(startButton);
//		panel.add(stopButton);
		panel.add(launchButton);
//		panel.add(msgArea);
//		panel.add(port);

		setTitle(WINDOW_LABEL);
		setSize(400, 200);
		setLocationRelativeTo(null);
		setDefaultCloseOperation(EXIT_ON_CLOSE);
	}

	private void updateStatusArea(String text) {
		statusArea.setText(text);
	}

	private void updateMsgArea(String text) {
		msgArea.setText(text);
	}
	
	private void initLayout(){
		quitButton = new JButton(QUIT_BUTTON);
		startButton = new JButton(START_BUTTON);
		stopButton = new JButton(STOP_BUTTON);
		launchButton = new JButton(LAUNCH);

		launchButton.setBounds(20, 30, 120, 50);
		quitButton.setBounds(20, 100, 120, 50);
		startButton.setBounds(100, 60, 120, 50);
		stopButton.setBounds(100, 130, 120, 50);

		statusArea = new JTextArea("this is a jtext area\nport: 8888");
		statusArea.setEditable(false);
		statusArea.setBounds(220, 30, 150, 50);

		msgArea = new JTextArea("No message.");
		msgArea.setEditable(false);
		msgArea.setBounds(300, 180, 200, 100);
		
		launchButton.setEnabled(true);
		
		startButton.setToolTipText(START_SERVER_TOOLTIP);
		stopButton.setToolTipText(STOP_SERVER_TOOLTIP);
		quitButton.setToolTipText(QUIT_TOOLTIP);
		launchButton.setToolTipText(LAUNCH_TOOLTIP_OFF);
		
		
	}

	private void bindActionListener() {

		quitButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				try {
					handler.stopServer();
				} catch (Exception ex) {

				} finally {
					System.exit(0);
				}
			}
		});

		startButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				try {
					handler.startServer();
				} catch (Exception e1) {
					e1.printStackTrace();
				}
				boolean on = handler.isServerRunning();
				launchButton.setEnabled(on);
				launchButton.setToolTipText(on?LAUNCH_TOOLTIP_ON:LAUNCH_TOOLTIP_OFF);
				updateStatusArea(getServerStatus());
			}

		});

		stopButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				try {
					handler.stopServer();
				} catch (Exception e1) {
					e1.printStackTrace();
				}
				boolean on = handler.isServerRunning();
				launchButton.setEnabled(on);
				launchButton.setToolTipText(on?LAUNCH_TOOLTIP_ON:LAUNCH_TOOLTIP_OFF);
				updateStatusArea(getServerStatus());
			}
		});

		launchButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				try {
					handler.startServer();
				} catch (Exception e2) {
					// TODO Auto-generated catch block
					e2.printStackTrace();
				}
				boolean on = handler.isServerRunning();
				if(on){
					String url = "http://localhost:" + handler.getPort()
							+ "/index.html";
					try {
						java.awt.Desktop.getDesktop().browse(
								java.net.URI.create(url));
					} catch (IOException e1) {
						updateMsgArea("Error launching the emulator.");
						e1.printStackTrace();
					}
				}
				updateStatusArea(getServerStatus());

			}
		});
	}

	private String getServerStatus() {
		int status = handler.getServerStatus();
		int port = handler.getPort();
		String serverStatus;

		switch (status) {
			case JettyServer.RUNNING:
				serverStatus = "Running";
				break;
			case JettyServer.OFFLINE:
				serverStatus = "Offline";
				break;
			case JettyServer.ERROR:
			default:
				serverStatus = "Error. Unknown status";
		}

		StringBuffer sb = new StringBuffer();
		sb.append("Server Status: " + serverStatus + "\n");

		if (status == JettyServer.RUNNING)
			sb.append("Port:          " + port + "\n");
		return sb.toString();
	}

	public static void main(String[] args) {
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				ControllerUI sg = new ControllerUI();
				sg.setVisible(true);
			}
		});
	}
}
