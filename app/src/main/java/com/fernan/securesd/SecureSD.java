package com.fernan.securesd;

import android.util.Log;

import java.io.IOException;

/**
 * Created by fernan on 2/11/16.
 */
public class SecureSD
{
    public static boolean getStatus()
    {
        Log.d("SecureSD", "Getting status...");
        try
        {
            String cmd = "/system/xbin/securesd status";
            Process proc = Runtime.getRuntime().exec(cmd);
            return (proc.waitFor() == 0) ? true : false;
        }
        catch (IOException ex)
        {
            Log.d("SecureSD", "updateMountState(): IOException");
        }
        catch (InterruptedException ex)
        {
            Log.d("SecureSD", "updateMountState(): InterruptedException");
        }
        return false;
    }

    public static boolean mount()
    {
        Log.d("SecureSD", "Mounting images...");
        try
        {
            String cmd = "/system/xbin/securesd mount";
            Process proc = Runtime.getRuntime().exec(cmd);
            return (proc.waitFor() == 0) ? true : false;
        }
        catch (IOException ex)
        {
            Log.d("SecureSD", "updateMountState(): IOException");
        }
        catch (InterruptedException ex)
        {
            Log.d("SecureSD", "updateMountState(): InterruptedException");
        }
        return false;
    }

    public static boolean unmount()
    {
        Log.d("SecureSD", "Unmounting images...");
        try
        {
            String cmd = "/system/xbin/securesd umount";
            Process proc = Runtime.getRuntime().exec(cmd);
            return (proc.waitFor() == 0) ? true : false;
        }
        catch (IOException ex)
        {
            Log.d("SecureSD", "updateMountState(): IOException");
        }
        catch (InterruptedException ex)
        {
            Log.d("SecureSD", "updateMountState(): InterruptedException");
        }
        return false;
    }
}
