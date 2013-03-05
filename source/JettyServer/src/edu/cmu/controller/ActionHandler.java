package edu.cmu.controller;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import edu.cmu.controller.TwoWaySerialComm;

/**
 * A handler class that handles connection with the serial COM port.
 * @author ziw
 *
 */
public class ActionHandler {
	private TwoWaySerialComm com;
	private static final String COM_PROT_NAME = "COM7";
	
	public ActionHandler(){
		//Do any initialization here.
		//constructor is called as soon as the server starts
	}
	
	/**
	 * Do something with buttonCode sent from ajax request. 
	 * Requires input validation.
	 * @param buttonCode
	 */
	public void handleButtonCode(String buttonName){
		if(buttonName == null || buttonName.trim().length()<=0)
			return;
		if(com == null){
//			try {
//				initSerialComm();
//			} catch (Exception e) {
//				e.printStackTrace();
//			}
			return;
		}
		if(buttonName.indexOf("init")!=-1){
			try {
				initialize();
			} catch (IOException e) {
				e.printStackTrace();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			return;
		}
		
		String decodedName;
		try {
			decodedName = URLDecoder.decode(buttonName, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			decodedName = "";
		}

		//Send to COM
		try {
			com.out.write(decodedName.getBytes());
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		catch(NullPointerException e){
			System.out.println("COM port not intialized.");
			e.printStackTrace();
		}
		System.out.println(decodedName);
		

		
	}
	
	/**
	 * Initialize the TwoWaySerialComm, if it has not been intialized.
	 * @throws Exception
	 */
	public void initSerialComm() throws Exception {
		if(com!=null)
			return;
		System.out.println("Initializing TwoWaySerialComm");
		com = new TwoWaySerialComm(COM_PROT_NAME);//this line takes a while
		System.out.println("SerialComm initialized");
	}
	
	
	/**
	 * Initialize the emulator. Start sending n's through the COM port to
	 * the BWT software
	 * @throws IOException
	 * @throws InterruptedException
	 */
	public void initialize() throws IOException, InterruptedException{
		ExecutorService executor = Executors.newCachedThreadPool();
		Future<?> reader = executor.submit(new SerialReader(com.in));
		executor.submit(new SerialWriter(com.out));
		try {
			reader.get(); //block until reader is done
		} catch (ExecutionException e) {
			e.printStackTrace();
		}
		executor.shutdownNow(); //kill writer
		while(!executor.isTerminated()); //block until writer is killed
		com.out.write("bt".getBytes());
		System.out.println("hand shaken");
		
	}

	
	public static class SerialReader implements Runnable {
        InputStream in;

        public SerialReader(InputStream in) {
            this.in = in;
        }

        public void run() {
            byte[] buffer = new byte[1024];
            byte[] miniBuffer = new byte[2]; //buffer for the incoming bytes
            								 // waiting for a b followed by a t
            int len = -1;
            boolean done = false;
            try {
                while (((len = this.in.read(buffer)) > -1) && (done == false)) {
                    for(int i = 0; i<len; i++){
                    	miniBuffer[0] = miniBuffer[1];
                    	miniBuffer[1] = buffer[i];
                    	if (((char)miniBuffer[0] == 'b') && ((char)miniBuffer[1] == 't')) {
                    		System.out.println("extending hand");
                    		return;
                    	}
                    }
                    
                }
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }
        
    }
	
	public static class SerialWriter implements Runnable{
        OutputStream out;

        public SerialWriter (OutputStream out) {
            this.out = out;
        }

        public void run() {
        	while(true){
        	
        		try {

                	System.out.print("n");
					this.out.write('n');
					this.out.write('n');
					this.out.write('n');
					this.out.write('n');
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
                
                try {
					Thread.sleep(100);
				} catch (InterruptedException e) {
					return;				
				}        	
            }
        }
	}



	
}
