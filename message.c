#include "headers.h"


void FreeMessage(void *msg)
{
	uMessage *m = (uMessage*)msg;

	if(m != NULL)
	{
		if(m->msg != NULL)
		{
			free(m->msg);
		}
		memset(m, 0x0, sizeof(uMessage));
		free(m);
	}
}

void update_last_message(uContext *ctx, char *strX, ...)
{
	va_list args;

	DLElement *e;
	uMessage *m;

	if(__msg_buffer == NULL)
	{
		return;
	}

	e = dlist_tail(ctx->lstMessages);
	if(e == NULL)
		return;

	m = dlist_data(e);

	strcpy(__msg_buffer, m->msg);
	free(m->msg);

	if(strX != NULL)
	{
		va_start(args, strX);
		// void gets around watcom binding error..
		vsprintf(__msg_buffer + strlen(__msg_buffer), strX, (void *)args);
		va_end(args);
	}
	else
	{
		__msg_buffer = "";
	}

	// cap message length
	__msg_buffer[screen_get_width()] = 0;
	ctx->redraw_messages = true;
	m->msg = strdup(__msg_buffer);
}

void add_msg(uContext *ctx, char *strX, ...)
{
	va_list args;
	uMessage *m;

	if(__msg_buffer == NULL)
	{
		return;
	}

	if(strX != NULL)
	{
		va_start(args, strX);
		// void gets around watcom binding error..
		vsprintf(__msg_buffer, strX, (void *)args);
		va_end(args);
	}
	else
	{
		__msg_buffer = "";
	}

	// cap message length
	__msg_buffer[screen_get_width()] = 0;

	if(dlist_size(ctx->lstMessages) > 0)
	{
		m = dlist_data(dlist_tail(ctx->lstMessages));

		// dont dupe last message needlessly
		if(m != NULL && stricmp(__msg_buffer, m->msg) == 0)
		{
			if(m->count < 250)
				m->count += 1;
			ctx->redraw_messages = true;
			return;
		}
	}

	ctx->redraw_messages = true;

	m = (uMessage*)calloc(1, sizeof(uMessage));
	assert(m != NULL);

	m->count = 1;
	m->msg = strdup(__msg_buffer);
	dlist_ins(ctx->lstMessages, m);

	while(dlist_size(ctx->lstMessages) > ctx->maxMessages)
	{
		dlist_remove(ctx->lstMessages, dlist_head(ctx->lstMessages), (void **)&m);
		FreeMessage(m);
	}
}
