package com.example.lift_tracking_prototype

import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import okhttp3.*
import java.io.IOException

class MainActivity : AppCompatActivity() {
    private val client = OkHttpClient()
    private val ESP32_URL = "http://192.168.23.136:80/setFloor"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.design_of_app)

        val currentFloorInput = findViewById<EditText>(R.id.current)
        val destinationFloorInput = findViewById<EditText>(R.id.destin)
        val requestButton = findViewById<Button>(R.id.button2)

        requestButton.setOnClickListener {
            val currentFloor = currentFloorInput.text.toString()
            val destinationFloor = destinationFloorInput.text.toString()
            if (currentFloor.isEmpty() || destinationFloor.isEmpty()) {
                Toast.makeText(this, "Please enter both floors", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }
            sendFloorData(currentFloor, destinationFloor)
        }
    }

    private fun sendFloorData(currentFloor: String, destinationFloor: String) {
        val url = "$ESP32_URL?current=$currentFloor&destination=$destinationFloor"
        Log.d("LiftRequest", "URL: $url")
        val request = Request.Builder()
            .url(url)
            .get()
            .build()

        client.newCall(request).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                Log.e("LiftRequest", "Failed to connect: ${e.message}", e)
                runOnUiThread {
                    Toast.makeText(this@MainActivity, "Failed to connect to ESP32: ${e.message}", Toast.LENGTH_SHORT).show()
                }
            }

            override fun onResponse(call: Call, response: Response) {
                if (!response.isSuccessful) {
                    Log.e("LiftRequest", "Server error: ${response.code}")
                    runOnUiThread {
                        Toast.makeText(this@MainActivity, "Server error: ${response.code}", Toast.LENGTH_SHORT).show()
                    }
                    return
                }

                val chosenLift = response.body?.string()?.trim() ?: "Unknown"
                Log.d("LiftRequest", "Chosen lift: $chosenLift")
                runOnUiThread {
                    updateLiftUI(chosenLift)
                }
            }
        })
    }

    private fun updateLiftUI(chosenLift: String) {
        val liftMap = mapOf(
            "A" to findViewById<TextView>(R.id.LiftA),
            "B" to findViewById<TextView>(R.id.LiftB),
            "C" to findViewById<TextView>(R.id.LiftC),
            "D" to findViewById<TextView>(R.id.LiftD)
        )

        val targetLiftView = liftMap[chosenLift]
        targetLiftView?.let { textView ->
            val handler = Handler(Looper.getMainLooper())
            val blinkDuration = 500L // 500 ms for each color change
            val totalBlinkTime = 5000L // 5 seconds in total
            val endTime = System.currentTimeMillis() + totalBlinkTime

            val blinkRunnable = object : Runnable {
                var isDarkGreen = true

                override fun run() {
                    if (System.currentTimeMillis() < endTime) {
                        textView.setBackgroundColor(
                            if (isDarkGreen) Color.parseColor("#f04f28") // Moderate dark green
                            else Color.parseColor("#6ed626") // Light green
                        )
                        isDarkGreen = !isDarkGreen
                        handler.postDelayed(this, blinkDuration)
                    } else {
                        textView.setBackgroundResource(R.drawable.circular_drawable) // Reset to orange
                    }
                }
            }

            handler.post(blinkRunnable)
        } ?: run {
            Toast.makeText(this, "Invalid Lift Selected", Toast.LENGTH_SHORT).show()
        }
    }
}
