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

Setup
=====
To Setup appdeploy clone the repo to your system

    git clone https://github.com/chipsnyder/appdeploy.git
Then run the Setup rake command.

    rake setup
Next run the command 

    appdeploy

If setup was successful you will see the following output

    Usage: appdeploy <command> [<options>]
    <path_to_app>                  : Local Path to .app file. ex /Users/me/Documents/CumberTest.app 
    <bundle_id>                    : Bundle Identification of application example. com.apple.Music 

    Commands:
      get_udid                     : Display UDID of connected device (will only show the first device discovered) 
      get_bundle_id <path_to_app>  : Display bundle identifier of app 
      install <path_to_app>        : Install app to device
      uninstall <bundle_id>        : Uninstall app by bundle id
      tunnel <from_port> <to_port> : Forward TCP connections to connected device
      list_apps [-p]               : Lists all installed apps on device
                 -p                : Include installation paths


    
<hr>
Partial Install
==========
Alternatively you can run a partial install by doing any of the following

    rake compile # compiles the source code. 
  The application can now be run by using 

    <path to compiled source>/appdeploy
To expose the compiled code the rest of the system preform the following command

    rake install 

<hr>
Uninstall
=======
To remove appdeploy from your system

    rake uninstall

<hr>
Usage Examples
=============

<h2>See Help</h2>

    appdeploy

    Usage: appdeploy <command> [<options>]
    <path_to_app>                  : Local Path to .app file. ex /Users/me/Documents/CumberTest.app 
    <bundle_id>                    : Bundle Identification of application example. com.apple.Music 

    Commands:
      get_udid                     : Display UDID of connected device (will only show the first device discovered) 
      get_bundle_id <path_to_app>  : Display bundle identifier of app 
      install <path_to_app>        : Install app to device
      uninstall <bundle_id>        : Uninstall app by bundle id
      tunnel <from_port> <to_port> : Forward TCP connections to connected device #not yet implemented
      list_apps [-p]               : Lists all installed apps on device
                 -p                : Include installation paths

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
<li>< path_to_app >  the path on your machine to the .app file of the compiled application. 
</ul>

     appdeploy get_bundle_id /Users/me/Projects/Sample.app

Your output will look something like
    
    com.apple.Sample

<h2>Install App</h2>
Install your compiled .app to the device

<b>Parameters:</b>
<ul>
<li>< path_to_app >  the path on your machine to the .app file of the compiled application. 
</ul>

    appdeploy install /Users/me/Projects/Sample.app

Your output will look something like

    /Users/me/Projects/Sample.app successfully installed.

<h2>Uninstall App</h2>
Uninstall your app from the device

<b>Parameters:</b>
<ul>
<li>< bundle_id >  the bundle id of the application to remove 
</ul> 

    appdeploy uninstall com.apple.Sample

 Your output will look something like

    com.apple.Sample successfully uninstalled.

<h2>List Apps</h2>
Lists all applications installed on the device. The list will provide each bundle id for the installed applications and not the name of the application it's self. You can also list all applications installed on the device including their installed location.   

<b>Parameters:</b>
<ul>
<li>-p  optionally display installed paths
</ul> 

     appdeploy list_apps

 Your output will look something like
    
    com.apple.AppStore
    com.google.ios.youtube
    com.apple.Maps
    com.apple.mobilesafari
    ...
To print out the installation paths for each application include the -p parameter
      
    appdeploy list_apps -p
    
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

<h2>Tunnel</h2>
Not Implemented yet Check back
