package com.voiceassist.api

import retrofit2.Response
import retrofit2.http.Body
import retrofit2.http.POST
import retrofit2.http.Query

/**
 * Service interface for interacting with the LLM API.
 */
interface LlmService {

    @POST("v1beta/models/gemini-1.5-flash-latest:generateContent")
    suspend fun generateContent(
        @Body request: GenerateContentRequest,
        @Query("key") apiKey: String
    ): Response<GenerateContentResponse>
}
