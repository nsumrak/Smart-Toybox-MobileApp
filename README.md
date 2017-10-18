# Smart Toybox Mobile App

This is the repository for the [Smart Toybox](https://github.com/nsumrak/Smart-Toybox-Embedded-TI) companion app. Currently only Android platform is fully supported.

## Requirements

1. This is a Cocos2d-x based project, so naturally Cocos2D-x and all its requirements or building Android app are required as well. We used version 3.8.1, which was latest at the time, on a Windows 10 machine. Follow the instructions on [Cocos2d-x website](http://www.cocos2d-x.org/) to set up Cocos for Android command line development:  
    <http://www.cocos2d-x.org/docs/static-pages/installation.html>  
    You can use Android Studio and Eclipse, but we will assume command line here.

    We found this tutorial very helpful:  
    <http://www.gamefromscratch.com/post/2014/09/29/Cocos2D-x-Tutorial-Series-Installation-Creating-a-Project-and-Hello-World.aspx>      
    It also explains further steps, like setting up the project and running the app.

2. Smart Toybox device with working software. 

3. Obviously, you will need an Android device. We've tested with various Android phones - Samsung Galaxy J7, OnePlus One, Elephone P9000, Nexus 5, Sony Xperia T, but there may be quirks with some models.

4. Lots of free space. In our case, Cocos2d-x installation took 2.5Gb, and cocos files that were copied to the project dir took additional 5Gb when compiled. There is a way to create Cocos2d-x project without copying all the files, by specifying the engine path when creating new cocos project. Check Cocos2d-x documentation if you want to do this.

## Set up the project

1. Create new Cocos2d-x project.

        $ cocos new -l cpp -d C:\path\to\your\project\dir Smart-Toybox-MobileApp
    Note: l in -l cpp is a small L, not number one

    Now you have created Cocos2d-x HelloWorld project. You may test it to see that everything is installed correctly. You can use steps for Test, Build and Deploy from below to test HelloWorl project in the same way you will afterwords run the Smart Toybox project.

2. Checkout this repo into the newly created project root. Keep in mind that git will not allow you to clone into a non-empty directory, so you will have to find another way. This is what worked for us:

        $ cd C:\path\to\your\project\dir\Smart-Toybox-MobileApp
        $ git init
        $ git remote add origin https://github.com/nsumrak/Smart-Toybox-MobileApp.git
        $ git fetch
        $ git reset origin/master
        $ git reset --hard HEAD
    
    Make sure that after this you have the correct version of the files – the ones from the repo, rather than the ones from the Cocos2d-x helloworld project.
    
## Test

You can now run the app on Windows from Visual studio. Open Visual Studio solution from proj.win32 folder in Visual Studio and try to run it. Some features will not work, since they are only implemented on Windows, but you can see the graphics and play with Demo mode.

## Build

    $ cd C:\path\to\your\project\dir\Smart-Toybox-MobileApp
    $ cocos compile -p android
   
It will take some time for the code to compile for the first time, but it'll be much faster later.

Cocos command line tool is very powerful and has many more options and commands that are more than useful. Be sure to check official guide here:  
<http://www.cocos2d-x.org/docs/editors_and_tools/cocosCLTool/>

## Deploy to Android device

Connect your phone to the PC and make sure USB debugging is enabled in Android. If you are on Windows, you may need to install Android driver for your phone. Check [Google's documentation](https://developer.android.com/studio/run/device.html) if you've never done it before.

The following command will install Smart Toybox mobile app on your mobile device. 

    $ cd C:\path\to\your\project\dir\Smart-Toybox-MobileApp
    $ cocos deploy -p android

Or you can deploy and run the app on your phone in one step:

    $ cocos run -p android

You can also use Android Debug Bridge (adb):

    $ adb install Smart-Toybox-MobileApp-debug.apk


## Third party code included [TODO]
* Cocos2d-x
* WavPack
* TI SmartConfig
