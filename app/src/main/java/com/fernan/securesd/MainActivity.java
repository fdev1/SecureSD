package com.fernan.securesd;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.ProgressBar;

public class MainActivity extends AppCompatActivity
{
    private boolean mounted = false;
    private Button mountButton;
    private ProgressBar progress;

    private void updateMountState()
    {
        if (mountButton.isInEditMode())
            return;
        mounted = SecureSD.getStatus();
        mountButton.setText(mounted ? "Unmount Secure SD" : "Mount Secure SD");
        mountButton.setEnabled(true);
        progress.setVisibility(View.INVISIBLE);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        mountButton = (Button) findViewById(R.id.button);
        progress = (ProgressBar) findViewById(R.id.progressBar);
        updateMountState();
    }

    public void OnMountClick(View v)
    {
        new ImageMounter().execute(new Object());
    }

    public class ImageMounter extends AsyncTask
    {
        @Override
        protected void onPreExecute()
        {
            super.onPreExecute();
            mountButton.setEnabled(false);
            progress.setVisibility(View.VISIBLE);
            progress.setIndeterminate(true);
        }

        @Override
        protected Object doInBackground(Object[] params)
        {
            if (mounted)
                SecureSD.unmount();
            else
                SecureSD.mount();
            return null;
        }

        @Override
        protected void onPostExecute(Object o)
        {
            super.onPostExecute(o);
            updateMountState();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings)
        {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}