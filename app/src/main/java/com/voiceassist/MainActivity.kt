package com.voiceassist

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.speech.RecognitionListener
import android.speech.RecognizerIntent
import android.speech.SpeechRecognizer
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.lifecycle.ViewModelProvider
import androidx.recyclerview.widget.LinearLayoutManager
import com.google.android.material.snackbar.Snackbar
import com.voiceassist.databinding.ActivityMainBinding
import com.voiceassist.ui.MessageAdapter
import com.voiceassist.viewmodel.MainViewModel
import java.util.Locale

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var viewModel: MainViewModel
    private lateinit var messageAdapter: MessageAdapter
    private lateinit var speechRecognizer: SpeechRecognizer
    
    // Added retry counter to prevent infinite retry loops
    private var recognizerRetryCount = 0
    private val MAX_RETRY_COUNT = 3
    private val RECORD_AUDIO_REQUEST_CODE = 101

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        // Initialize ViewModel
        viewModel = ViewModelProvider(this)[MainViewModel::class.java]
        
        // Set up RecyclerView
        setupRecyclerView()
        
        // Set up speech recognizer
        setupSpeechRecognizer()
        
        // Set up voice button
        binding.voiceButton.setOnClickListener {
            if (checkPermission()) {
                if (viewModel.isListening.value == true) {
                    stopListening()
                } else {
                    startListening()
                }
            } else {
                requestPermission()
            }
        }
        
        // Observe ViewModel state
        observeViewModel()
    }
    
    private fun setupRecyclerView() {
        messageAdapter = MessageAdapter()
        binding.conversationRecyclerView.apply {
            layoutManager = LinearLayoutManager(this@MainActivity).apply {
                stackFromEnd = true
            }
            adapter = messageAdapter
        }
    }
    
    private fun setupSpeechRecognizer() {
        speechRecognizer = SpeechRecognizer.createSpeechRecognizer(this)
        speechRecognizer.setRecognitionListener(object : RecognitionListener {
            override fun onReadyForSpeech(params: Bundle?) {
                viewModel.setListening(true)
                binding.statusText.setText(R.string.listening)
                binding.voiceProgress.visibility = View.VISIBLE
                binding.voiceButton.setImageResource(android.R.drawable.ic_media_pause)
                // Reset retry counter when recognition starts successfully
                recognizerRetryCount = 0
            }

            override fun onBeginningOfSpeech() {
                binding.voiceProgress.progress = 0
            }

            override fun onRmsChanged(rmsdB: Float) {
                val scaledProgress = (rmsdB * 10).toInt().coerceIn(0, 100)
                binding.voiceProgress.progress = scaledProgress
            }

            override fun onBufferReceived(buffer: ByteArray?) {}

            override fun onEndOfSpeech() {
                stopListening()
            }

            override fun onError(error: Int) {
                // Log the error but don't show it to the user unless it's persistent
                when (error) {
                    SpeechRecognizer.ERROR_AUDIO -> {
                        // Just restart listening for audio errors
                        binding.voiceProgress.postDelayed({
                            if (viewModel.isListening.value == true) {
                                recreateSpeechRecognizer()
                                startListening()
                            }
                        }, 500)
                    }
                    SpeechRecognizer.ERROR_CLIENT -> {
                        // Handle client side error by restarting the recognizer after a short delay
                        if (recognizerRetryCount < MAX_RETRY_COUNT) {
                            recognizerRetryCount++
                            binding.voiceProgress.postDelayed({
                                if (viewModel.isListening.value == true) {
                                    recreateSpeechRecognizer()
                                    startListening()
                                }
                            }, 800)
                        } else {
                            // Only show error after multiple failures
                            stopListening()
                            // Reset retry count for next attempt
                            recognizerRetryCount = 0
                        }
                    }
                    SpeechRecognizer.ERROR_INSUFFICIENT_PERMISSIONS -> {
                        stopListening()
                        requestPermission()
                    }
                    SpeechRecognizer.ERROR_NETWORK,
                    SpeechRecognizer.ERROR_NETWORK_TIMEOUT -> {
                        // Silently retry for network issues
                        binding.voiceProgress.postDelayed({
                            if (viewModel.isListening.value == true) {
                                startListening()
                            }
                        }, 1000)
                    }
                    SpeechRecognizer.ERROR_NO_MATCH -> {
                        // If no match is found, restart listening without showing an error
                        binding.voiceProgress.postDelayed({
                            if (viewModel.isListening.value == true) {
                                startListening()
                            }
                        }, 300)
                    }
                    SpeechRecognizer.ERROR_RECOGNIZER_BUSY -> {
                        // If recognizer is busy, release it and create a new one
                        recreateSpeechRecognizer()
                        binding.voiceProgress.postDelayed({
                            if (viewModel.isListening.value == true) {
                                startListening()
                            }
                        }, 300)
                    }
                    SpeechRecognizer.ERROR_SERVER -> {
                        // Retry once for server errors, then show error
                        if (recognizerRetryCount < 1) {
                            recognizerRetryCount++
                            binding.voiceProgress.postDelayed({
                                if (viewModel.isListening.value == true) {
                                    startListening()
                                }
                            }, 1000)
                        } else {
                            stopListening()
                            recognizerRetryCount = 0
                        }
                    }
                    SpeechRecognizer.ERROR_SPEECH_TIMEOUT -> {
                        // Just restart listening for speech timeouts
                        binding.voiceProgress.postDelayed({
                            if (viewModel.isListening.value == true) {
                                startListening()
                            }
                        }, 300)
                    }
                    else -> {
                        // For unknown errors, retry silently
                        binding.voiceProgress.postDelayed({
                            if (viewModel.isListening.value == true) {
                                recreateSpeechRecognizer()
                                startListening()
                            }
                        }, 500)
                    }
                }

                // Don't show Snackbar errors at all - they're causing the blinking
                // Instead log the error for debugging purposes
                println("Speech recognition error: $error")
            }

            override fun onResults(results: Bundle?) {
                val matches = results?.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION)
                if (!matches.isNullOrEmpty()) {
                    val recognizedText = matches[0]
                    viewModel.processVoiceInput(recognizedText)
                }
            }

            override fun onPartialResults(partialResults: Bundle?) {}

            override fun onEvent(eventType: Int, params: Bundle?) {}
        })
    }
    
    /**
     * Recreates the speech recognizer to fix busy errors
     */
    private fun recreateSpeechRecognizer() {
        if (::speechRecognizer.isInitialized) {
            speechRecognizer.destroy()
        }
        speechRecognizer = SpeechRecognizer.createSpeechRecognizer(this)
        setupSpeechRecognizer()
    }

    private fun startListening() {
        if (!SpeechRecognizer.isRecognitionAvailable(this)) {
            Toast.makeText(this, "Speech recognition is not available on this device", Toast.LENGTH_LONG).show()
            return
        }
        
        // Clear any previous recognizer to avoid issues
        if (::speechRecognizer.isInitialized) {
            speechRecognizer.destroy()
        }
        setupSpeechRecognizer()

        // Show user that app is listening
        binding.statusText.setText(R.string.listening)
        binding.voiceProgress.visibility = View.VISIBLE
        Toast.makeText(this, "Listening... Please speak now", Toast.LENGTH_SHORT).show()

        val intent = Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH).apply {
            putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM)
            putExtra(RecognizerIntent.EXTRA_LANGUAGE, Locale.getDefault())
            putExtra(RecognizerIntent.EXTRA_MAX_RESULTS, 3)
            putExtra(RecognizerIntent.EXTRA_PARTIAL_RESULTS, true)
            // Increase timeout values to give more time for speech
            putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_MINIMUM_LENGTH_MILLIS, 300L)
            putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_COMPLETE_SILENCE_LENGTH_MILLIS, 1500L)
            putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS, 1500L)
            // Try with both online and offline recognition
            putExtra(RecognizerIntent.EXTRA_PREFER_OFFLINE, false)
        }
        
        try {
            viewModel.setListening(true)
            speechRecognizer.startListening(intent)
            println("DEBUG: Speech recognizer started")
        } catch (e: Exception) {
            Toast.makeText(this, "Speech recognition error: ${e.message}", Toast.LENGTH_LONG).show()
            println("DEBUG: Speech recognition failed to start: ${e.message}")
            viewModel.setListening(false)
            binding.voiceProgress.visibility = View.INVISIBLE
        }
    }
    
    private fun stopListening() {
        speechRecognizer.stopListening()
        viewModel.setListening(false)
        binding.statusText.setText(R.string.tap_to_speak)
        binding.voiceProgress.visibility = View.INVISIBLE
        binding.voiceButton.setImageResource(R.drawable.ic_mic)
    }
    
    private fun observeViewModel() {
        viewModel.conversationHistory.observe(this) { messages ->
            messageAdapter.submitList(messages)
            if (messages.isNotEmpty()) {
                binding.conversationRecyclerView.smoothScrollToPosition(messages.size - 1)
            }
        }
        
        viewModel.isProcessing.observe(this) { isProcessing ->
            binding.statusText.setText(if (isProcessing) R.string.processing else R.string.tap_to_speak)
            binding.voiceButton.isEnabled = !isProcessing
        }
        
        viewModel.errorEvent.observe(this) { errorMessage ->
            errorMessage?.let {
                Snackbar.make(binding.root, it, Snackbar.LENGTH_LONG).show()
                viewModel.errorHandled()
            }
        }
    }
    
    private fun checkPermission(): Boolean {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED
    }
    
    private fun requestPermission() {
        ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.RECORD_AUDIO), RECORD_AUDIO_REQUEST_CODE)
    }
    
    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == RECORD_AUDIO_REQUEST_CODE && grantResults.isNotEmpty()) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(this, "Permission Granted", Toast.LENGTH_SHORT).show()
            } else {
                Toast.makeText(this, "Permission Denied", Toast.LENGTH_SHORT).show()
            }
        }
    }
    
    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        return true
    }
    
    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_clear -> {
                viewModel.clearConversation()
                true
            }
            R.id.action_settings -> {
                // TODO: Navigate to settings screen
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        if (::speechRecognizer.isInitialized) {
            speechRecognizer.destroy()
        }
    }
}
