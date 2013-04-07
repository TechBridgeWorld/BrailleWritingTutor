package edu.cmu.scripting;

import java.io.File;
import java.io.FilenameFilter;


import com.google.gson.Gson;

/**
 * A class used to find and load user scripts that reside
 * in ./BWT_SCRIPTS folder
 * @author ziw
 *
 */
public class ScriptLoader {

	private static final String SCRIPT_FOLDER_PATH = "./BWT_SCRIPTS";
	private static final String SCRIPT_EXTENSION = ".bwt";

	private File scriptsFolder;//the root folder that holds all scripts
	
	public ScriptLoader(){
		checkAndCreateFolder();
	}

	private void checkAndCreateFolder(){
		File f = new File(SCRIPT_FOLDER_PATH);
		if(!f.exists() ||  !f.isDirectory()){
			f.mkdir();
		}
		scriptsFolder = f;
	}
	
	
	
	/**
	 * Return a JSON string representing an array of files names 
	 * in ./BWT_SCRIPTS folder that end in .bwt
	 * @return
	 */
	public String getAllScriptNames(){
		String[] allScripts = scriptsFolder.list(new FilenameFilter(){

			@Override
			public boolean accept(File dir, String name) {
				return name!= null && name.endsWith(SCRIPT_EXTENSION);
			}
			
		});
		Gson g = new Gson();
		return g.toJson(allScripts);
	}
	
	/**
	 * Load and parse the script with the given name.
	 * Return error messages if there are errors or a JSON string
	 * representing a recording queue.
	 * @param scriptName
	 * @return
	 */
	public String loadScript(String scriptName){
		return ScriptParser.parseScript(SCRIPT_FOLDER_PATH+"/"+scriptName);
	}	
	
	
	
}

