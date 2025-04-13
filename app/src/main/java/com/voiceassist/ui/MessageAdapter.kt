package com.voiceassist.ui

import android.text.format.DateFormat
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.cardview.widget.CardView
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.voiceassist.R
import com.voiceassist.model.Message
import com.voiceassist.model.Sender
import java.util.Date

/**
 * Adapter for displaying messages in a RecyclerView.
 */
class MessageAdapter : ListAdapter<Message, MessageAdapter.MessageViewHolder>(MessageDiffCallback()) {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): MessageViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.item_message, parent, false)
        return MessageViewHolder(view)
    }

    override fun onBindViewHolder(holder: MessageViewHolder, position: Int) {
        val message = getItem(position)
        holder.bind(message)
    }

    class MessageViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val senderNameTextView: TextView = itemView.findViewById(R.id.senderName)
        private val messageTextView: TextView = itemView.findViewById(R.id.messageText)
        private val messageCard: CardView = itemView.findViewById(R.id.messageCard)
        private val timestampTextView: TextView = itemView.findViewById(R.id.timestampText)

        fun bind(message: Message) {
            when (message.sender) {
                Sender.USER -> {
                    senderNameTextView.text = itemView.context.getString(R.string.you)
                    messageCard.setCardBackgroundColor(itemView.context.getColor(R.color.userMessage))
                    
                    // Align user messages to the right
                    val layoutParams = messageCard.layoutParams as ViewGroup.MarginLayoutParams
                    layoutParams.marginEnd = 0
                    layoutParams.marginStart = itemView.resources.getDimensionPixelSize(R.dimen.message_margin)
                    messageCard.layoutParams = layoutParams
                }
                Sender.ASSISTANT -> {
                    senderNameTextView.text = itemView.context.getString(R.string.assistant)
                    messageCard.setCardBackgroundColor(itemView.context.getColor(R.color.assistantMessage))
                    
                    // Align assistant messages to the left
                    val layoutParams = messageCard.layoutParams as ViewGroup.MarginLayoutParams
                    layoutParams.marginStart = 0
                    layoutParams.marginEnd = itemView.resources.getDimensionPixelSize(R.dimen.message_margin)
                    messageCard.layoutParams = layoutParams
                }
            }

            messageTextView.text = message.text
            timestampTextView.text = formatTimestamp(message.timestamp)
            
            // Set the loading state if the message is being processed
            if (message.isProcessing) {
                messageTextView.setText(R.string.processing)
            }
        }
        
        private fun formatTimestamp(date: Date): String {
            return DateFormat.format("h:mm a", date).toString()
        }
    }

    /**
     * DiffUtil callback for efficient RecyclerView updates.
     */
    class MessageDiffCallback : DiffUtil.ItemCallback<Message>() {
        override fun areItemsTheSame(oldItem: Message, newItem: Message): Boolean {
            return oldItem.id == newItem.id
        }

        override fun areContentsTheSame(oldItem: Message, newItem: Message): Boolean {
            return oldItem == newItem
        }
    }
} 