// UCLA CS 111 Lab 1 command reading


#include "command.h"
#include "command-internals.h"
#include "alloc.h"


#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define debug

typedef struct c_stream *c_stream_t;

/* constant definition */
#define INIT_LENGTH 4

/* function declaration */
void init_c_stream(c_stream_t new_stream); // initialize an empty command stream
int is_word(char c); // check if a character is a word (word definition is in the lab manual)
int is_special_token(char); // check if a character is a special character (8 tokens in the lab manual)
int is_comment(char); // check if char is #
int is_white_space(char); // check if white space but NOT new line

// TODO: sub task, should be easy
command_t make_simple_command(c_stream_t* s); // make a simple command from the command stream passed in
command_t make_and_command(command_t a1, command_t a2); // make and command
command_t make_or_command(command_t o1, command_t o2); // make or command
command_t make_pipe_command(command_t p1, command_t p2); // make pipeline command
command_t make_sequence_command(command_t s1, command_t s2); // make sequence command
command_t make_subshell_command(command_t sub); // make subshell command 

// TODO: main tasks, do as the TA said in the discussion 
//			 see more detail in functions implementation below, you can
//			 change the type and number of argument as necessary
command_t read_simple_command(c_stream_t* s);
command_t read_and_or_command(c_stream_t* s);
command_t read_pipeline_command(c_stream_t* s);
command_t read_sequence_command(c_stream_t* s);
command_t read_subshell_command(c_stream_t* s);
 
/* end function declaration section */

// helper functions
enum command_type read_command_type(char *command);



/* debug function */
void print_cmd_stream(c_stream_t s);

c_stream_t* current;

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  // TODO:
  // 1. create a c_stream object using alloc
  // 2. populate the object based on the get_next_byte(get_next_byte_argument)
  // 3. store a command line until \n
  // 4. keep track of the number of lines while looping trough the file_stream
  // 5. Ignored the comments and the empty lines
	c_stream_t head_stream = NULL;
	c_stream_t tail_stream = NULL;
  c_stream_t new_stream; 
	//checked_malloc(sizeof(struct c_stream));
  //init_c_stream(new_stream);


  int line_num = 1;
  char c = get_next_byte(get_next_byte_argument);
  
	//printf("c is %c\n", c);
  while (c != EOF)
  {
    // ignoring beginning white space
    while((c == ' ') || (c == '\t'))
		{
			c = get_next_byte(get_next_byte_argument);
		}
		// ignore comment, read until see an end-of-line
		if (c == '#')
		{
			//printf("Comment\n");
			while (c != '\n')
				c = get_next_byte(get_next_byte_argument);
		}

		int max_size = INIT_LENGTH;
		int current_pos = 0;
		size_t resize = max_size;
		// read everything else
		// printf("Reading %c\n", c);
		if (is_word(c))
		{	
			new_stream = checked_malloc(sizeof(struct c_stream));
  		init_c_stream(new_stream);
			while(is_word(c))
			{
				if(current_pos == max_size) // text too big, need grow in size
				{
					max_size = max_size * 2;
					resize = max_size *sizeof(char);
					new_stream->head = checked_grow_alloc(new_stream->head, &resize);
				}
				new_stream->head[current_pos] = c;
				current_pos++;
				c = get_next_byte(get_next_byte_argument);
			}
			
			new_stream->line_num = line_num;
			new_stream->head[current_pos] = 0;
			new_stream->token_size = current_pos+1;
			new_stream->prev = NULL;
			new_stream->next = NULL;
			// add new stream to the end of result stream
			if(head_stream == NULL)
				tail_stream = head_stream = new_stream;
			else
			{
				tail_stream->next = new_stream;
				new_stream->prev = tail_stream;
				tail_stream = new_stream;
			}
			
#ifdef debug
			//printf("New stream's text: %s\n", new_stream->head);
#endif
			// reverse len and position
			max_size = INIT_LENGTH;
			current_pos = 0;
		}
		else if(is_special_token(c)) // deal with special token
		{
			new_stream = checked_malloc(sizeof(struct c_stream));
  		init_c_stream(new_stream);
			if(current_pos == max_size) // text too big, need grow in size
			{
				max_size = max_size * 2;
				resize = max_size *sizeof(char);
				new_stream->head = checked_grow_alloc(new_stream->head, &resize);
			}
			new_stream->head[current_pos] = c;
			current_pos++;
			//c = get_next_byte(get_next_byte_argument);
			if(c == '&')
			{
				printf("Read %c\n",c);
				// read next char, if not another &, output error
				c = get_next_byte(get_next_byte_argument);
				printf("Next char %c\n",c);
				if(c != '&')
				{
					error(1,0, "Syntax error, missing & at linenum %i",line_num);
					//c = get_next_byte(get_next_byte_argument);
					//continue;
				}
				else
				{
					//c = get_next_byte(get_next_byte_argument);
					new_stream->head[current_pos] = c;
					current_pos++;
				}
				c = get_next_byte(get_next_byte_argument);
			}
			else if(c == '|')
			{
				printf("Read %c\n",c);
				// read next char, if not another | or not a word, output error
				c = get_next_byte(get_next_byte_argument);
				printf("Next char %c\n",c);
				if(c != '|' && c != ' ' && c != '\n' && c != '\t' && !is_word(c))
				{
					error(1,0, "Syntax error, | at linenum %i",line_num);
					//c = get_next_byte(get_next_byte_argument);
					//continue;
				}
				else if (c == '|')
				{
					//c = get_next_byte(get_next_byte_argument);
					new_stream->head[current_pos] = c;
					current_pos++;
					c = get_next_byte(get_next_byte_argument); 
				}
			}
			else // other special char
			{
				// read next char, if not a word, output error
				c = get_next_byte(get_next_byte_argument);
				if(c != ' ' && c != '\n' && c != '\t' && !is_word(c))
				{
					error(1,0, "Syntax error, special char %c follow by unrecognized symbol at linenum %i", c,  line_num);
					//c = get_next_byte(get_next_byte_argument);
					//continue;
				}
				else
				{
					
				}
			}
			new_stream->line_num = line_num;
			new_stream->head[current_pos] = 0;
			new_stream->token_size = current_pos+1;
			new_stream->prev = NULL;
			new_stream->next = NULL;
			// add new stream to the end of result stream
			if(head_stream == NULL)
				tail_stream = head_stream = new_stream;
			else
			{
				tail_stream->next = new_stream;
				new_stream->prev = tail_stream;
				tail_stream = new_stream;
			}

			// reverse len and position
			max_size = INIT_LENGTH;
			current_pos = 0;
		}
		else if (c == '\n') // new line, increase line number
		{
			line_num++;
			c = get_next_byte(get_next_byte_argument);
		}
		else if (!is_word(c) && !is_special_token(c) && c != ' ' && c != '\n' && c != '\t' && c != EOF)
		{
			error(1, 0, "%i: unrecognized symbol %c.", line_num, c);
		}	
  }

	
	//current = &head_stream;
	//current = 0;
	command_stream_t result = checked_malloc(sizeof(struct command_stream));
	result->s = head_stream;
	//command_stream_t result = &st;
  return result;
}


command_t
read_command_stream (command_stream_t s)
{
  // TODO: 
  // 1. Check if the c_stream_t s is END or not, if it is, return NULL
  // 2. If the command stream is not END, then process our logic with the stream
  // 3. Return the result
  printf("Using the read_c_stream.\n");
	if(s->s == NULL)
	{
		//printf("NULL\n");
		return 0;
	}
	command_t cmd;
	
	cmd = read_and_or_command(&(s->s));
  /* printf("c_stream max size: %d.\n", s->max_size); */
	if(s == NULL)
	{	
		//printf("NULL\n");
		cmd = 0;
	}
  return cmd;
}

void
init_c_stream(c_stream_t new_stream)
{
  new_stream->head = (char*)checked_malloc(INIT_LENGTH * sizeof(char));
  new_stream->line_num = 0;
  new_stream->size = 0;
  new_stream->next = NULL;
  new_stream->prev = NULL;
}

int
is_word(char c)
{
  return isalpha(c) || isdigit(c) || (c == '!') || (c == '%') || (c == '+') || (c == ',')
                        || (c == '-') || (c == '.') || (c == '/') || (c == ':') || (c == '@') || (c == '^')
                        || (c == '_');
}

int 
is_special_token(char c) // check if a character is a special character (8 tokens in the lab manual)
{
  return (c == '|') || (c == '&') || (c == ';') || (c == '(') || (c == ')') || (c == '<') || (c == '>');
}

int 
is_comment(char c) // check if char is #
{
  return (c == '#');
}


/* TODO TASK HERE */
command_t 
read_and_or_command(c_stream_t* s)
{
	// 1. read the pipeline command until && or || or end
  command_t cmd = read_pipeline_command(s);
  //Debug
  int is_and_or_cmd = 100;
	if ((*s) != NULL) 
		is_and_or_cmd = read_command_type((*s)->head);
	if((*s) != NULL)
	{
  	if (is_and_or_cmd == AND_COMMAND || is_and_or_cmd == OR_COMMAND) 
			{
    	// 2. If the next command is && or ||, then make and_or command
    	// Read the next command
    	(*s) = (*s)->next;
    	command_t next_cmd = read_and_or_command(s);
    	command_t and_or_cmd;
    	if (is_and_or_cmd == AND_COMMAND) {
      	cmd = make_and_command(cmd, next_cmd);
    	} else if (is_and_or_cmd == OR_COMMAND){
      	cmd = make_or_command(cmd, next_cmd);
    	}
    	//return and_or_cmd;	
  	}
	}
return cmd;
}
command_t 
read_pipeline_command(c_stream_t* s)
{
	// read the subshell command
	command_t cmd = NULL;
  //command_t cmd = read_subshell_command(s); // comment for testing, should uncomment it later
	command_t cmd_left = read_simple_command(s); // read the left of the pipe
  //(*s) = (*s)->next;
  // Debug
#ifdef debug
	//if((*s) != NULL)
  //	printf("read_pipeline_command: next head: %s\n", (*s)->head);
#endif
	if((*s) != NULL)
	{
		// read the pipe symbol and error checking
		if (strcmp((*s)->head, "|") != 0) // next symbol is not pipe
		{
			//error(1,0, "Read_pipeline_command: Syntax error at linenum %i", (*s)->line_num);
		}
		else // read the right pipe and create pipe command
		{
			(*s) = (*s)->next;
			command_t cmd_right = read_pipeline_command(s); // read the right of the pipe
			cmd_left = make_pipe_command(cmd_left, cmd_right); 
		}
	}

	return cmd_left;
}
command_t 
read_simple_command(c_stream_t* s)
{
	// NOT COMPLETE
	// need to deal with < > simple
	// this is the lowest lever command
	//printf("read_simple_command: next head: %s\n", (*s)->head);
  command_t cmd = make_simple_command(s);
  //s = s->next;
  
	
	c_stream_t tmp = (*s);
		
	if(tmp != NULL && strcmp(tmp->head, "<") == 0) // input
	{
		//(*s) = (*s)->next;
		if(!is_word((*s)->next->head[0])) // after < is not a word
		{
			error(1,0, "Read_simple_command:Syntax error at linenum %i", (*s)->next->line_num);
		}
		cmd->input = checked_malloc(2*sizeof(char));
		strcpy(cmd->input, (*s)->next->head);
		//printf("input =%s\n", cmd->input);
		(*s) = (*s)->next;
		if((*s) != NULL)
			(*s) = (*s)->next;
		else
			error(1,0, "Unexpected error");
		tmp = (*s);
	}
	if(tmp != NULL && strcmp(tmp->head, ">") == 0) // output
	{
		//(*s) = (*s)->next;
		if(!is_word((*s)->next->head[0])) // after < is not a word
		{
			error(1,0, "Read_symbol_command:Syntax error at linenum %i", (*s)->next->line_num);
		}
		cmd->output = checked_malloc(2*sizeof(char));
		strcpy(cmd->output, (*s)->next->head);
		//printf("output =%s\n", cmd->output);
		(*s) = (*s)->next;
		if((*s) != NULL)
			(*s) = (*s)->next;
		else
			error(1,0, "Unexpected error");
	}
	
  //Debug
	/*
	if((*s) != NULL)
  	printf("read_simple_command: next head: %s\n", (*s)->head);
	else
		printf("read_simple_command:NULL\n");
	*/
	return cmd;
}

command_t 
read_sequence_command(c_stream_t* s)
{
	// same as above
	return 0;
}
command_t 
read_subshell_command(c_stream_t* s)
{
	// same as above
	return 0;
}

// sub tasks
command_t 
make_simple_command(c_stream_t* s) 
{
	//printf("make_simple_command: next head: %s\n", (*s)->head);
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = SIMPLE_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	// get the word
	int max_size = INIT_LENGTH;
	int current_pos = 0;
	size_t resize = max_size;
	
	result->u.word = (char**)checked_malloc(sizeof(char*) * max_size);

	int line_num = (*s)->line_num;
	while((*s) != NULL && line_num == (*s)->line_num)
	{
		//printf("s is not NULL\n");
		char c = (*s)->head[0];
		if(is_word(c))
		{
			//printf("Read %s\n", (*s)->head);
			if(current_pos == max_size) // text too big, need grow in size
				{
					max_size = max_size * 2;
					resize = max_size *sizeof(char);
					result->u.word = checked_grow_alloc(result->u.word, &resize);
				}
			result->u.word[current_pos] = (char*)checked_malloc((*s)->token_size * sizeof(char));
			result->u.word[current_pos] = memcpy(result->u.word[current_pos], (*s)->head, (*s)->token_size);
			current_pos++;
			//printf("Word after copied %s\n",result->u.word[current_pos-1]);
		}
		else // not word, break the loop
		{		
			//printf("Not word: Read %s\n", (*s)->head);
			if(current_pos == max_size) // text too big, need grow in size
				{
					max_size = max_size * 2;
					resize = max_size *sizeof(char);
					result->u.word = checked_grow_alloc(result->u.word, &resize);
				}
			result->u.word[current_pos] = 0;
			break;
		}
		if((*s) != NULL)
			(*s) = (*s)->next;
		//else
			//printf("Next token %s\n", (*s)->head);
	}
	return result;
} 
command_t 
make_and_command(command_t a1, command_t a2)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = AND_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.command[0] = a1;
	result->u.command[1] = a2;
	
	return result;
}
command_t 
make_or_command(command_t o1, command_t o2)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = OR_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.command[0] = o1;
	result->u.command[1] = o2;
	return result;
}
command_t 
make_pipe_command(command_t p1, command_t p2)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = PIPE_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.command[0] = p1;
	result->u.command[1] = p2;
	return result;
}
command_t 
make_sequence_command(command_t s1, command_t s2)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = SEQUENCE_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.command[0] = s1;
	result->u.command[1] = s2;

	return result;
}
command_t 
make_subshell_command(command_t sub)
{
	command_t result = (command_t)checked_malloc(sizeof(struct command));
	result->type = AND_COMMAND;
	result->status = -1;
	result->input = NULL;
	result->output = NULL;
	
	result->u.subshell_command = sub;
	
	return result;
}

enum command_type read_command_type(char *command) {
  if (*command == 0) {
    return -1; //exit
  }
  else if (strcmp(command, "&&") == 0) {
    return AND_COMMAND;
  }
  else if (strcmp(command, "||") == 0) {
    return OR_COMMAND;
  }
  else if (*command == ';') {
    return SEQUENCE_COMMAND;
  }
  else if (*command == '|') {
    return PIPE_COMMAND;
  }
  else if (*command == '(') {
    // TODO: improve this, dont know how to match subshell correctly
    return SUBSHELL_COMMAND;
  }

  return SIMPLE_COMMAND;
}
/* END TODO 		 */


/* debug function */
void print_cmd_stream(c_stream_t s)
{
	int line_num = 1;
	while (s != NULL)
	{
		if(s->line_num != line_num)
		{
			printf("\n");
			line_num = s->line_num;
		}
		printf("%s ",s->head);
		s = s->next;
		
	}
	printf("\n");
}
