appdeploy
=========

App Deploy is an extension of Apple's MobileDevice private framework, designed to assist in the development and automated testing of iOS applications. <hr>

Requirements
===========
<ul>
<li>Xcode 4 or later
<li>iOS Device with an installed development certificate and provisioning profile (this can be tested by launching the application from Xcode)
</ul>
<hr>

Setup and Installation
======================
To Setup appdeploy clone the repo to your system

    git clone https://github.com/chipsnyder/appdeploy.git
Then run the Setup rake command.

    rake setup
Next run the command 

    appdeploy

If setup was successful you will see the following output
	
	Usage: appdeploy <command> [<options>]
    	<path_to_app>
        	- Local Path to .app file. ex /Users/me/Documents/CumberTest.app 

    	<bundle_id>
        	- Bundle Identification of application. ex com.apple.Music 

    	<file_path>
        	- The path to the file on the device. ex /Documents/File.png 

    	<destination_path>
        	- The local path to store the downloaded file. ex /Users/me/File.png 

	Commands:
    	get_udid
        	- Display UDID of connected device (will only show the first device discovered) 

    	get_bundle_id <path_to_app>
        	- Display bundle identifier of app 

    	install <path_to_app>
        	- Install app to device

    	uninstall <bundle_id>
        	- Uninstall app by bundle id

    	remove_file <bundle_id> <file_path>
        	- Deletes the specified file at the given path

    	download_file <bundle_id> <file_path> <destination_path>
        	- Deletes the specified file at the given path
        	
       	upload_file <bundle_id> <file_path> <destination_path>
        	- Upload the specified file at the given path

    	list_files <bundle_id> [-verbose]
        	- Lists all of the files in the sandbox for the specified app.
        	- Use the optional -verbose paramater to get also list all directories

    	list_apps [-paths]
        	- Lists all installed apps on device
        	- Use the optional -paths paramater to include all application installation paths

    
<hr>
Installation
============
Alternatively you can run a partial install by doing any of the following

    rake compile # compiles the source code. 
  The application can now be run by using 

    <path to compiled source>/appdeploy
To expose the compiled code the rest of the system preform the following command

    rake install 

<hr>
Uninstall
=========
To remove appdeploy from your system

    rake uninstall

<hr>
Usage Examples
==============

<h2>See Help</h2>
To see the help simply run
	
	appdeploy
	
Your output should look like:

    Usage: appdeploy <command> [<options>]
    	<path_to_app>
        	- Local Path to .app file. ex /Users/me/Documents/CumberTest.app 

    	<bundle_id>
        	- Bundle Identification of application. ex com.apple.Music 

    	<file_path>
        	- The path to the file on the device. ex /Documents/File.png 

    	<destination_path>
        	- The local path to store the downloaded file. ex /Users/me/File.png 

	Commands:
    	get_udid
        	- Display UDID of connected device (will only show the first device discovered) 

    	get_bundle_id <path_to_app>
        	- Display bundle identifier of app 

    	install <path_to_app>
        	- Install app to device

    	uninstall <bundle_id>
        	- Uninstall app by bundle id

    	remove_file <bundle_id> <file_path>
        	- Deletes the specified file at the given path

    	download_file <bundle_id> <file_path> <destination_path>
        	- Deletes the specified file at the given path
        	
    	upload_file <bundle_id> <file_path> <destination_path>
        	- Upload the specified file at the given path

    	list_files <bundle_id> [-verbose]
        	- Lists all of the files in the sandbox for the specified app.
        	- Use the optional -verbose paramater to get also list all directories

    	list_apps [-paths]
        	- Lists all installed apps on device
        	- Use the optional -paths paramater to include all application installation paths

<h2>Get UDID</h2>

This command will return the UDID of the connected device. Note if multiple devices are detected the call will only return the UDID of the first discovered device.

    appdeploy get_udid

Your output will look something like 

    2be702beae2ac34fc0d7f8ae2b5b808a402fc01a

<b>Note:</b> The above output is a sample UDID from Apple's Documentation and not a true device known to me.

<h2>Get Bundle ID</h2>
This will return the bundle id for the specified application. 

<b>Parameters:</b>
<ul>
<li><b>< path_to_app ></b>  the path on your machine to the .app file of the compiled application. 
</ul>

     appdeploy get_bundle_id /Users/me/Projects/Sample.app

Your output will look something like
    
    com.apple.Sample

<h2>Install App</h2>
Install your compiled .app to the device

<b>Parameters:</b>
<ul>
<li><b>< path_to_app ></b>  the path on your machine to the .app file of the compiled application. 
</ul>

    appdeploy install /Users/me/Projects/Sample.app

Your output will look something like

    /Users/me/Projects/Sample.app successfully installed.

<h2>Uninstall App</h2>
Uninstall your app from the device

<b>Parameters:</b>
<ul>
<li><b>< bundle_id ></b>  the bundle id of the application to remove 
</ul> 

    appdeploy uninstall com.apple.Sample

 Your output will look something like

    com.apple.Sample successfully uninstalled.
    
<h2>Remove File</h2>
Delete the file or directory from your device. 

<b>-Note:</b> The directory must be empty before it will be removed.

<b>Parameters:</b>
<ul>
<li><b>< bundle_id ></b>  The bundle id of the target application
<li><b>< file_path ></b>  The path to the file on the device.
</ul> 

    appdeploy remove_file com.apple.Sample /Documents/File.png

 Your output will look something like

    /Documents/File.png successfully deleted.

<h2>Download File</h2>
Download a file from the device to your machine. 

<b>Parameters:</b>
<ul>
<li><b>< bundle_id ></b>  The bundle id of the target application
<li><b>< file_path ></b>  The path to the file on the device.
<li><b>< destination_path ></b>  The local path to store the downloaded file.
</ul> 

    appdeploy download_file com.apple.Sample /Documents/File.png /Users/me/Documents/fileCopy.png

 Your output will look something like

    /Documents/File.png successfully downloaded to /Users/me/Documents/fileCopy.png.
    
<h2>Upload File</h2>
Upload a file from the device to your machine. 

<b>Parameters:</b>
<ul>
<li><b>< bundle_id ></b>  The bundle id of the target application
<li><b>< file_path ></b>  The path to the file on the device.
<li><b>< destination_path ></b>  The local path to store the downloaded file.
</ul> 

    appdeploy upload_file com.apple.Sample /Users/me/Documents/fileCopy.png /Documents/File.png

 Your output will look something like

    /Users/me/Documents/fileCopy.png successfully uploaded to /Documents/File.png


<h2>List Files</h2>
Lists all files inside the Documents directory of the Application. The List will include the full path to each file.

<b>Parameters:</b>
<ul>
<li><b>< bundle_id ></b>  the bundle id of the application to inspect
<li><b>-verbose</b>  optionally display all directories separately
</ul> 

    appdeploy list_files com.apple.Sample -verbose

 Your output will look something like

 	/Documents/coreDataFile.data
    /Documents/SubFolder/File.jpeg
    /Documents/SubFolder/File1.txt
    /Documents/SubFolder/SubFolder2/File2.pdf
    ...
 
 With optional paramater you will also see
 
 	/Documents/SubFolder/SubFolder3
 	...

<h2>List Apps</h2>
Lists all applications installed on the device. The list will provide each bundle id for the installed applications and not the name of the application it's self. You can also list all applications installed on the device including their installed location.   

<b>Parameters:</b>
<ul>
<li><b>-paths</b>  optionally display installed paths
</ul> 

     appdeploy list_apps

 Your output will look something like
    
    com.apple.AppStore
    com.google.ios.youtube
    com.apple.Maps
    com.apple.mobilesafari
    ...
To print out the installation paths for each application include the -p parameter
      
    appdeploy list_apps -paths
    
 Your output will look something like
    
    com.apple.AppStore
        Path:  /Applications/AppStore.app
    com.google.ios.youtube
        Path: /private/var/mobile/Applications/XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX/YouTube.app
    com.apple.Maps
        Path: /Applications/Maps.app
    com.apple.mobilesafari
	    Path: /private/var/mobile/Applications/XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX/MobileSafari.app
       ...

<hr>
Compile Your Project
================
Appdeploy relies on having a compiled version of your project. To compile the code from command line you can use Apple's xcodebuild tool.

<b>Parameters:</b>
<ul>
<li><b>-project</b>  The path the .xcodeproj file on your machine
<li><b>-target</b> The 
</ul> 

    xcodebuild -project /Users/me/Projects/Sample.xcodeproj -target Sample -sdk iphoneos -configuration Debug clean build 

The compiled application will be in the following be at the path (Note ./ represents the location of the folder where the .xcodeproj folder lives)
     
    ./build/Debug-iphoneos/Sample.app

You can find more information at: 
<a href="https://developer.apple.com/library/mac/documentation/Darwin/Reference/Manpages/man1/man.1.html#//apple_ref/doc/man/1/man"> https://developer.apple.com/library/mac/documentation/Darwin/Reference/Manpages/man1/man.1.html#//apple_ref/doc/man/1/man</a>

<hr>
License
======

The MIT License (MIT)<br>
Copyright (c) 2013 Chip Snyder<br><br>
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:<br><br>
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.<br><br>
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

<a href="http://opensource.org/licenses/MIT">The MIT License (MIT)</a>

 
