/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2014 Chukong Technologies Inc.
 
http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
package org.cocos2dx.cpp;

import org.cocos2dx.lib.Cocos2dxActivity;

import android.os.Build;
import android.os.Vibrator;
import android.annotation.TargetApi;
import android.content.Context;
import android.util.Log;
import android.os.Bundle;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.media.AudioTrack;
import android.media.AudioManager;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import java.net.InetAddress;
import java.util.ArrayList;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import com.integrity_project.smartconfiglib.SmartConfig;
import com.integrity_project.smartconfiglib.SmartConfigListener;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class AppActivity extends Cocos2dxActivity {

    private static volatile Cocos2dxActivity mainActivity;

	static String SERVICE_NAME = "smarttoybox";
    static String SERVICE_TYPE = "_toybox._tcp.";

	static NsdManager mNsdManager = null;
	static boolean dns_enable = false;
	
	static WifiManager mWifiManager = null;

	static ArrayList<String> boxlist = new ArrayList<String>(); 


    public static void setMainActivity(Cocos2dxActivity activity)
    {
        mainActivity = activity;
    }
 
    public static Cocos2dxActivity getMainActivity()
    {
        if(mainActivity == null)
        {
            Log.w("VIBRO", "Warning : null main Activity");
        }
        return mainActivity;
    }
 
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        AppActivity.setMainActivity(this);
		mNsdManager = (NsdManager) getSystemService(Context.NSD_SERVICE);
		//if(dns_enable) startDnsSD(); // Will be called with onResume
		mWifiManager = (WifiManager)getSystemService(Context.WIFI_SERVICE);
    }

	@Override
	protected void onPause() {
		if (mNsdManager != null && dns_enable)
			mNsdManager.stopServiceDiscovery(mDiscoveryListener);

		stopRecording();
		super.onPause();
	}
    
	@Override
	protected void onResume() {
		super.onResume();
		if (mNsdManager != null && dns_enable) startDnsSD();
	}
    
	@Override
	protected void onDestroy() {
		if (mNsdManager != null) {
			mNsdManager.stopServiceDiscovery(mDiscoveryListener);
		}
		super.onDestroy();
	}
    

	///////////////////
	// Audio recording


	static short[] buffer = new short[160000];
	static int readed = 0;
	static AudioRecord audioRecord = null;
	static Thread thread = null;
	static boolean started = false;

	static int recordAudio()
	{
		if(audioRecord != null) stopRecording();

		thread = new Thread(new Runnable() {
			public void run() {
				try {
					readed = 0;
					int bufferSize = AudioRecord.getMinBufferSize(16000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT); 

					audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 16000, 
							AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize); 

					audioRecord.startRecording();
					Log.d("AudioRecord", "recording started");

					while (started && readed < 160000) {
						int toread = 160000-readed;
						if(toread > 1024) toread = 1024;
						int bufferReadResult = audioRecord.read(buffer, readed, toread);
						if(bufferReadResult >= 0) {
							readed += bufferReadResult;
							Log.d("AudioRecord", "got buffer");
						}
					}

					audioRecord.stop();
					Log.d("AudioRecord", "recording stopped");

				} catch (Throwable t) {
					Log.e("AudioRecord", "Recording Failed");
				}
				started = false;
			}
		});

		Vibrator v = (Vibrator) getMainActivity().getSystemService(Context.VIBRATOR_SERVICE);
		if (getMainActivity().getSystemService(Context.VIBRATOR_SERVICE) != null)
		{
			v.vibrate(100);
		}

		started = true;
		thread.start();

		return 0;
	}

	static int stopRecording()
	{
		if(thread == null) {
			Log.e("AudioRecord", "stop recording but null");
			return 0;
		}
		Log.d("AudioRecord", "stop recording");
		started = false;
		try {
			thread.join();
		} catch (Throwable t) {
			Log.e("AudioRecord", "stop recording error");
		}
		audioRecord.release();
		audioRecord = null;
		thread = null;
		return readed;
	}

	static short[] getRecBuffer()
	{
		return buffer;
	}

	static AudioTrack player = null;

	static int playAudio(short[] buf)
	{
		if(player != null) stopAudioPlay();
		try {
			player = new AudioTrack(AudioManager.STREAM_MUSIC, 16000, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT,
							buf.length*2, AudioTrack.MODE_STATIC);
			player.write(buf, 0, buf.length);
			player.play();
		} catch (Throwable t) {
			Log.e("AudioPlay", "play audio error");
			return 1;
		}
		return 0;
	}

	static void stopAudioPlay()
	{
		if(player != null) {
			player.stop();
			player.release();
			player = null;
		}
	}


	////////////////
	// DNS-SD stuff


	static NsdManager.DiscoveryListener mDiscoveryListener = new NsdManager.DiscoveryListener() {
		//  Called as soon as service discovery begins.
		@Override
		public void onDiscoveryStarted(String regType) {
			Log.d("DNS-SD", "Service discovery started");
		}

		@Override
		public void onServiceFound(NsdServiceInfo service) {
			// A service was found!  Do something with it.
			Log.d("DNS-SD", "Service discovery success! " + service);
			if (!service.getServiceType().equals(SERVICE_TYPE)) {
				// Service type is the string containing the protocol and
				// transport layer for this service.
				Log.d("DNS-SD", "Unknown Service Type: " + service.getServiceType());
			}
			//else if (service.getServiceName().equals(mServiceName)) {
				// The name of the service tells the user what they'd be
				// connecting to. It could be "Bob's Chat App".
			//	Log.d("DNS-SD", "Same machine: " + mServiceName);
			//}
			else if (service.getServiceName().toLowerCase().contains(SERVICE_NAME)){
				Log.d("DNS-SD", "YES! it is toybox service " + service.getServiceName());
				mNsdManager.resolveService(service, mResolveListener);
			}
		}

		@Override
		public void onServiceLost(NsdServiceInfo service) {
			// When the network service is no longer available.
			// Internal bookkeeping code goes here.
			
			InetAddress host = service.getHost();
			if(host != null) boxlist.remove(host.toString());
			Log.e("DNS-SD", "service lost" + service);
		}

		@Override
		public void onDiscoveryStopped(String serviceType) {
			Log.i("DNS-SD", "Discovery stopped: " + serviceType);
		}

		@Override
		public void onStartDiscoveryFailed(String serviceType, int errorCode) {
			Log.e("DNS-SD", "Discovery failed: Error code:" + errorCode);
			mNsdManager.stopServiceDiscovery(this);
		}

		@Override
		public void onStopDiscoveryFailed(String serviceType, int errorCode) {
			Log.e("DNS-SD", "Discovery failed: Error code:" + errorCode);
			mNsdManager.stopServiceDiscovery(this);
		}
	};


	static NsdManager.ResolveListener mResolveListener = new NsdManager.ResolveListener() {
		@Override
		public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
			// Called when the resolve fails.  Use the error code to debug.
			Log.e("DNS-SD", "Resolve failed" + errorCode);
		}

		@Override
		public void onServiceResolved(NsdServiceInfo serviceInfo) {
			Log.e("DNS-SD", "Resolve Succeeded. " + serviceInfo);

			//if (serviceInfo.getServiceName().equals(mServiceName)) {
			//	Log.d("DNS-SD", "Same IP.");
			//	return;
			//}
			int port = serviceInfo.getPort();
			InetAddress host = serviceInfo.getHost();
			if (host != null && !boxlist.contains(host.toString())) {
				boxlist.add(host.toString());
				Log.d("DNS-SD", "Resolved toyboxservice on IP="+host.toString()+" port " + port);
			}
		}
	};


	static void startDnsSD()
	{
		dns_enable = true;
		mNsdManager.discoverServices(SERVICE_TYPE, NsdManager.PROTOCOL_DNS_SD, mDiscoveryListener);
	}

	static void stopDnsSD()
	{
		if(mNsdManager == null) return;
		mNsdManager.stopServiceDiscovery(mDiscoveryListener);
		dns_enable = false;
	}

	static int getDnsSDdiscoveryNum()
	{
		if(mNsdManager == null) return 0;
		return boxlist.size();
	}

	static String getDnsSDdiscoveryItem(int i)
	{
		if(mNsdManager == null || i<0 || i>=boxlist.size()) return new String();
		return boxlist.get(i);
	}

	// Smart Config
	static byte[] freeData;
	static SmartConfig smartConfig;
	static SmartConfigListener smartConfigListener;

	public static String ipToString(int i) {
		return (i & 0xFF ) + "." +
				((i >> 8) & 0xFF) + "." +
				((i >> 16 ) & 0xFF) + "." +
				((i >> 24) & 0xFF) ;
	}
	public static String getGateway() {
		return mWifiManager != null ? ipToString(mWifiManager.getDhcpInfo().gateway) : null;
	}

	public static String getSSID() {
		if (mWifiManager != null) {
			WifiInfo wi =  mWifiManager.getConnectionInfo();
			if (wi != null && wi.getNetworkId() != -1) {
				String ssid = wi.getSSID();
				int lastCharIndex = ssid.length() - 1;
				return  (ssid.charAt(0) == '\"' && ssid.charAt(lastCharIndex) == '\"')
						? ssid.substring(1, lastCharIndex) : ssid;
			}
		}
		return null;
	}

	static void startSmartConfig(String ssid, String pwd) {
		Log.d("SmartConfig", "in smart config");
		String gateway = getGateway();
		//byte[] paddedEncryptionKey;
		//paddedEncryptionKey = (ENCKEY + SmartConfigConstants.ZERO_PADDING_16).substring(0, 16).trim().getBytes();

//		if (smartconfig_device_name_field.getText().length() > 0) { // device name isn't empty
//			byte[] freeDataChars = new byte[smartconfig_device_name_field.getText().length() + 2];
//			freeDataChars[0] = 0x03;
//			freeDataChars[1] = (byte) smartconfig_device_name_field.getText().length();
//			for (int i=0; i<smartconfig_device_name_field.getText().length(); i++) {
//				freeDataChars[i+2] = (byte) smartconfig_device_name_field.getText().charAt(i);
//			}
//			freeData = freeDataChars;
//		} else {
			freeData = new byte[1];
			freeData[0] = 0x03;
//		}
		Log.d("SmartConfig", "ssid: " + ssid + " pwd: " + pwd + " gw: " + gateway);
		smartConfig = null;
		smartConfigListener = new SmartConfigListener() {
			@Override
			public void onSmartConfigEvent(SmtCfgEvent event, Exception e) { Log.d("SmartConfig", "SmartConfig event"); }
		};
		try {
			smartConfig = new SmartConfig(smartConfigListener, freeData, pwd, null, gateway, ssid, (byte) 0, "");
			if (smartConfig != null) smartConfig.transmitSettings();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	static void stopSmartConfig() {
		try {
			if (smartConfig != null) smartConfig.stopTransmitting();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
