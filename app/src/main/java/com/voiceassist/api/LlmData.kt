package com.voiceassist.api

import com.google.gson.annotations.SerializedName

data class GenerateContentRequest(
    val contents: List<Content>
)

data class Content(
    val role: String,
    val parts: List<Part>
)

data class Part(
    val text: String
)

data class GenerateContentResponse(
    val candidates: List<Candidate>,
    val promptFeedback: PromptFeedback? = null
)

data class Candidate(
    val content: Content,
    val finishReason: String? = null,
    val index: Int? = null,
    val safetyRatings: List<SafetyRating>? = null
)

data class PromptFeedback(
    val blockReason: String? = null,
    val safetyRatings: List<SafetyRating>
)

data class SafetyRating(
    val category: String,
    val probability: String
)
