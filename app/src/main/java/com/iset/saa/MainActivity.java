package com.iset.saa;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import androidx.activity.EdgeToEdge;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {

    DatabaseReference dashDataRef;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        TextView temperature = findViewById(R.id.temperature);
        TextView humidite = findViewById(R.id.humidite);
        TextView humiditeSol = findViewById(R.id.humiditeSol);

        ToggleButton motopompe = findViewById(R.id.motopompe);
        ToggleButton vanne1 = findViewById(R.id.vanne1);
        ToggleButton vanne2 = findViewById(R.id.vanne2);

        dashDataRef = FirebaseDatabase.getInstance().getReference("data");

        motopompe.setOnClickListener(v -> dashDataRef.child("motopompe").setValue(motopompe.isChecked()));
        vanne1.setOnClickListener(v -> dashDataRef.child("vanne1").setValue(vanne1.isChecked()));
        vanne2.setOnClickListener(v -> dashDataRef.child("vanne2").setValue(vanne2.isChecked()));

        dashDataRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                DashData data = dataSnapshot.getValue(DashData.class);

                temperature.setText(String.valueOf(data.temperature) + "°");
                humidite.setText(String.valueOf(data.humidite) + "%");
                humiditeSol.setText(String.valueOf(data.humiditeSol) + "%");

                motopompe.setChecked(Boolean.TRUE.equals(data.motopompe));
                vanne1.setChecked(Boolean.TRUE.equals(data.vanne1));
                vanne2.setChecked(Boolean.TRUE.equals(data.vanne2));
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {
                Log.w("firebase", "error", error.toException());
            }

        });

    }

}