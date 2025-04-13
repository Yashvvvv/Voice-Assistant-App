package com.voiceassist.api

import com.voiceassist.model.Message
import retrofit2.Response
import retrofit2.http.Body
import retrofit2.http.Headers
import retrofit2.http.POST

/**
 * Service interface for interacting with the LLM API.
 */
interface LlmService {
    
    @Headers("Content-Type: application/json")
    @POST("chat/completions")
    suspend fun getChatCompletion(@Body request: ChatCompletionRequest): Response<ChatCompletionResponse>
}

/**
 * Request model for chat completion API call.
 */
data class ChatCompletionRequest(
    val model: String,
    val messages: List<ChatMessage>,
    val temperature: Float = 0.7f,
    val max_tokens: Int = 150
)

/**
 * Represents a message in the chat API format.
 */
data class ChatMessage(
    val role: String, // "system", "user", or "assistant"
    val content: String
)

/**
 * Response model for chat completion API call.
 */
data class ChatCompletionResponse(
    val id: String,
    val object: String,
    val created: Long,
    val model: String,
    val choices: List<Choice>,
    val usage: Usage
)

/**
 * Represents a choice from the chat completion API.
 */
data class Choice(
    val index: Int,
    val message: ChatMessage,
    val finish_reason: String
)

/**
 * Represents token usage information.
 */
data class Usage(
    val prompt_tokens: Int,
    val completion_tokens: Int,
    val total_tokens: Int
) 