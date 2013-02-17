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

public class ControllerUI extends JFrame {

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

	public ControllerUI() {
		initUI();
		handler = new UIActionHandler();
		updateStatusArea(getServerStatus());
	}

	public void initUI() {
		JPanel panel = new JPanel();
		getContentPane().add(panel);

		panel.setLayout(null);

		quitButton = new JButton(QUIT_BUTTON);
		startButton = new JButton(START_BUTTON);
		stopButton = new JButton(STOP_BUTTON);
		launchButton = new JButton(LAUNCH);

		
		launchButton.setBounds(100, 200, 120, 50);
		quitButton.setBounds(100, 270, 120, 50);
		startButton.setBounds(100, 60, 120, 50);
		stopButton.setBounds(100, 130, 120, 50);

		statusArea = new JTextArea("this is a jtext area\nport: 8888");
		statusArea.setEditable(false);
		statusArea.setBounds(300, 60, 200, 100);

		msgArea = new JTextArea("No error message.");
		msgArea.setEditable(false);
		msgArea.setBounds(300, 180, 200, 100);

		

		
		quitButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				try{
					handler.stopServer();
				}
				catch(Exception ex){
					
				}
				finally{
					System.exit(0);
				}
			}

		});

		startButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				handler.startServer();
				updateStatusArea(getServerStatus());
			}

		});

		stopButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				handler.stopServer();
				updateStatusArea(getServerStatus());
			}
		});

		launchButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				
				String url = "http://localhost:"+handler.getPort()+"/haha.html";
				try {
					java.awt.Desktop.getDesktop().browse(java.net.URI.create(url));
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					
					e1.printStackTrace();
				}
			}
		});

		panel.add(statusArea);
		panel.add(quitButton);
		panel.add(startButton);
		panel.add(stopButton);
		panel.add(launchButton);
		panel.add(msgArea);
		

		setTitle(WINDOW_LABEL);
		setSize(600, 400);
		setLocationRelativeTo(null);
		setDefaultCloseOperation(EXIT_ON_CLOSE);
	}

	private void updateStatusArea(String text){
		statusArea.setText(text);
	}
	
//	private void updateMsgArea(String text){
//		msgArea.setText(text);
//	}
	
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
