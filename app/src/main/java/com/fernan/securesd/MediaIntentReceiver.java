package com.fernan.securesd;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

/**
 * Created by fernan on 2/11/16.
 */
public class MediaIntentReceiver extends BroadcastReceiver
{
    public void onReceive(Context context, Intent intent)
    {
        if (intent.ACTION_MEDIA_EJECT.equals(intent.getAction()))
        {
            if (SecureSD.getStatus())
                if (!SecureSD.unmount())
                    Log.d("MediaIntentReceiver", "Unmount failed");
            Log.d("MediaIntentReceiver", "Ejecting SD card...");
        }
        else if (intent.ACTION_MEDIA_UNMOUNTED.equals(intent.getAction()))
        {
            Log.d("MediaIntentReceiver", "Unmounted SD card...");
        }
        else if (intent.ACTION_MEDIA_MOUNTED.equals(intent.getAction()))
        {
            if (!SecureSD.getStatus())
                if (!SecureSD.mount())
                    Log.d("MediaIntentReceiver", "Mount failed");
            Log.d("MediaIntentReceiver", "Mounted SD card...");
        }
    }
}