package com.voiceassist.api

import com.voiceassist.BuildConfig
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
    private val apiKey: String = BuildConfig.GEMINI_API_KEY
    private val baseUrl = "https://generativelanguage.googleapis.com/"

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
            val contents = mutableListOf<Content>()

            // Add conversation history
            conversationHistory.forEach { message ->
                val role = when (message.sender) {
                    Sender.USER -> "user"
                    Sender.ASSISTANT -> "model"
                }
                contents.add(Content(role = role, parts = listOf(Part(text = message.text))))
            }

            // Add the current user message
            contents.add(Content(role = "user", parts = listOf(Part(text = userMessage))))

            val request = GenerateContentRequest(contents = contents)

            val response = llmService.generateContent(request, apiKey)

            if (response.isSuccessful && response.body() != null) {
                val responseBody = response.body()!!

                if (responseBody.candidates.isNullOrEmpty()) {
                    responseBody.promptFeedback?.let {
                        val blockReason = it.blockReason ?: "Unknown reason"
                        return@withContext Result.failure(Exception("Request blocked by API for safety reasons: $blockReason"))
                    }
                    return@withContext Result.failure(Exception("No response from assistant"))
                }

                if (responseBody.candidates.isNotEmpty() && responseBody.candidates[0].content.parts.isNotEmpty()) {
                    Result.success(responseBody.candidates[0].content.parts[0].text)
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
