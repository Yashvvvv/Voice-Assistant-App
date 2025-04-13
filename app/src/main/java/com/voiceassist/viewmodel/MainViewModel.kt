package com.voiceassist.viewmodel

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.voiceassist.api.LlmRepository
import com.voiceassist.model.Message
import com.voiceassist.model.Sender
import kotlinx.coroutines.launch

/**
 * ViewModel for the main screen, handling conversation state and LLM interactions.
 */
class MainViewModel : ViewModel() {
    
    private val llmRepository = LlmRepository()
    
    private val _conversationHistory = MutableLiveData<List<Message>>(emptyList())
    val conversationHistory: LiveData<List<Message>> = _conversationHistory
    
    private val _isListening = MutableLiveData<Boolean>(false)
    val isListening: LiveData<Boolean> = _isListening
    
    private val _isProcessing = MutableLiveData<Boolean>(false)
    val isProcessing: LiveData<Boolean> = _isProcessing
    
    private val _errorEvent = MutableLiveData<String?>(null)
    val errorEvent: LiveData<String?> = _errorEvent
    
    /**
     * Processes a voice input from the user.
     */
    fun processVoiceInput(voiceInput: String) {
        if (voiceInput.isBlank()) return
        
        // Add user message to conversation
        val userMessage = Message(
            text = voiceInput,
            sender = Sender.USER
        )
        addMessageToConversation(userMessage)
        
        // Add a placeholder for the assistant response
        val assistantPlaceholder = Message(
            text = "",
            sender = Sender.ASSISTANT,
            isProcessing = true
        )
        addMessageToConversation(assistantPlaceholder)
        
        // Set processing state
        _isProcessing.value = true
        
        // Get response from LLM
        viewModelScope.launch {
            val currentConversation = _conversationHistory.value ?: emptyList()
            // Exclude the placeholder message from the context sent to the LLM
            val contextForLLM = currentConversation.dropLast(1)
            
            val result = llmRepository.getAssistantResponse(voiceInput, contextForLLM)
            
            // Update conversation with real response
            result.fold(
                onSuccess = { responseText ->
                    // Replace the placeholder with the actual response
                    val updatedConversation = currentConversation.toMutableList()
                    updatedConversation[updatedConversation.size - 1] = Message(
                        text = responseText,
                        sender = Sender.ASSISTANT
                    )
                    _conversationHistory.value = updatedConversation
                },
                onFailure = { error ->
                    // Handle error
                    _errorEvent.value = error.message ?: "Unknown error"
                    
                    // Remove the placeholder message
                    val updatedConversation = currentConversation.toMutableList()
                    updatedConversation.removeAt(updatedConversation.size - 1)
                    _conversationHistory.value = updatedConversation
                }
            )
            
            // End processing state
            _isProcessing.value = false
        }
    }
    
    /**
     * Sets the listening state.
     */
    fun setListening(isListening: Boolean) {
        _isListening.value = isListening
    }
    
    /**
     * Clears the conversation history.
     */
    fun clearConversation() {
        _conversationHistory.value = emptyList()
    }
    
    /**
     * Adds a message to the conversation history.
     */
    private fun addMessageToConversation(message: Message) {
        val currentList = _conversationHistory.value ?: emptyList()
        _conversationHistory.value = currentList + message
    }
    
    /**
     * Clears the error event after it has been handled.
     */
    fun errorHandled() {
        _errorEvent.value = null
    }
} 