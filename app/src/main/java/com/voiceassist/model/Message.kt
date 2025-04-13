package com.voiceassist.model

import java.util.Date

/**
 * Represents a message in the conversation between the user and the assistant.
 */
data class Message(
    val id: String = java.util.UUID.randomUUID().toString(),
    val text: String,
    val sender: Sender,
    val timestamp: Date = Date(),
    val isProcessing: Boolean = false
)

/**
 * Represents the sender of a message.
 */
enum class Sender {
    USER,
    ASSISTANT
} 