package com.voiceassist.api

import com.voiceassist.model.Message
import com.voiceassist.model.Sender
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory

/**
 * Repository class to handle LLM API operations.
 */
class LlmRepository {
    private val apiKey: String = "" // TODO: Securely store and retrieve API key
    private val baseUrl = "https://api.openai.com/v1/"
    private val defaultModel = "gpt-3.5-turbo"
    
    private val retrofit: Retrofit = Retrofit.Builder()
        .baseUrl(baseUrl)
        .addConverterFactory(GsonConverterFactory.create())
        .build()
    
    private val llmService: LlmService = retrofit.create(LlmService::class.java)
    
    /**
     * Gets a chat completion response from the LLM service.
     */
    suspend fun getAssistantResponse(
        userMessage: String,
        conversationHistory: List<Message>
    ): Result<String> = withContext(Dispatchers.IO) {
        try {
            // Convert app Message objects to ChatMessage objects
            val messages = mutableListOf<ChatMessage>()
            
            // Add a system message to define the assistant's behavior
            messages.add(ChatMessage(
                role = "system",
                content = "You are a helpful assistant that provides concise and accurate information."
            ))
            
            // Add conversation history
            conversationHistory.forEach { message ->
                val role = when (message.sender) {
                    Sender.USER -> "user"
                    Sender.ASSISTANT -> "assistant"
                }
                messages.add(ChatMessage(role = role, content = message.text))
            }
            
            // Add the current user message
            messages.add(ChatMessage(role = "user", content = userMessage))
            
            // Create the request
            val request = ChatCompletionRequest(
                model = defaultModel,
                messages = messages
            )
            
            // Make the API call
            val response = llmService.getChatCompletion(request)
            
            if (response.isSuccessful && response.body() != null) {
                val responseBody = response.body()!!
                if (responseBody.choices.isNotEmpty()) {
                    Result.success(responseBody.choices[0].message.content)
                } else {
                    Result.failure(Exception("No response from assistant"))
                }
            } else {
                Result.failure(Exception("API Error: ${response.errorBody()?.string()}"))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
} 