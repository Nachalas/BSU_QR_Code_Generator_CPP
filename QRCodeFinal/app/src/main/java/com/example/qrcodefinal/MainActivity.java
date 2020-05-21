package com.example.qrcodefinal;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.PictureDrawable;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.caverock.androidsvg.SVG;
import com.caverock.androidsvg.SVGImageView;
import com.caverock.androidsvg.SVGParseException;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("main");
    }

    private EditText et;
    private SVGImageView svgImageView;
    private SVG svg = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        et = findViewById(R.id.text);
        svgImageView = findViewById(R.id.imageView1);
        Button button = findViewById(R.id.button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String toEncode = et.getText().toString();
                String svgString = Main(toEncode);
                try{
                    svg = SVG.getFromString(svgString);
                    svgImageView.setSVG(svg);
                } catch(SVGParseException e) {
                    e.printStackTrace();
                }

            }
        });

        Button saveButton = findViewById(R.id.save_button);
        saveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                        != PackageManager.PERMISSION_GRANTED) {
                    String[] permissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE};
                    ActivityCompat.requestPermissions(MainActivity.this, permissions, 1);
                }

                if(svg != null) {
                    PictureDrawable drawable = new PictureDrawable(svg.renderToPicture());
                    Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(),drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
                    Canvas canvas = new Canvas(bitmap);
                    canvas.drawPicture(drawable.getPicture());
                    try {
                        File folder = new File(Environment.getExternalStorageDirectory().getPath() + "/QRGen");
                        if (!folder.exists()) {
                            folder.mkdirs();
                        }
                        String uniqueString = UUID.randomUUID().toString();
                        bitmap.compress(Bitmap.CompressFormat.JPEG, 100,  new FileOutputStream(folder + "/" + uniqueString +".jpg"));
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    }
                } else {
                    Toast toast = Toast.makeText(getApplicationContext(), "Generate a QR code first", Toast.LENGTH_SHORT);
                    toast.show();
                }

            }
        });

        findViewById(R.id.delete_button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                et.setText("");
            }
        });


    }

    public native String Main(String input);
}
