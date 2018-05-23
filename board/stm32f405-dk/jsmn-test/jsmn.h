#ifndef __JSMN_H_
#define __JSMN_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef JSMN_COMPACT
#define JSMN_COMPACT 0
#endif

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
enum {
	JSMN_PRIMITIVE = 0,
	JSMN_OBJECT = 1,
	JSMN_ARRAY = 2,
	JSMN_STRING = 3
};

typedef enum {
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3,
} jsmnerr_t;

/**
 * JSON token description.
 * @param		type	type (object, array, string etc.)
 * @param		start	start position in JSON data string
 * @param		end		end position in JSON data string
 */
#if 0
typedef struct {
	jsmntype_t type;
	int start;
	int end;
	int size;
#ifdef JSMN_PARENT_LINKS
	int parent;
#endif
} jsmntok_t;
#endif



#if JSMN_COMPACT

#ifdef JSMN_PARENT_LINKS
#error "JSMN_PARENT_LINKS conflicts with JSMN_COMPACT!"
#endif

#define JSMN_NULL 0xfff

typedef struct {
	unsigned int size: 6; /* up to 31 elements per object/array */
	unsigned int type: 2;
	unsigned int start: 12; /* up to 4096 bytes JSON file */
	unsigned int end: 12;
} jsmntok_t;

#else 

#define JSMN_NULL 0xffff

typedef struct {
	unsigned short type: 2;
	unsigned short size: 14; /* up to 8192 elements per object/array */
	unsigned short start; /* up to 64K JSON file */
	unsigned short end;
#ifdef JSMN_PARENT_LINKS
	short parent;
#endif
} jsmntok_t;

#endif

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
	unsigned int pos; /* offset in the JSON string */
	unsigned int toknext; /* next token to allocate */
	int toksuper; /* superior token node, e.g parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
jsmnerr_t jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		jsmntok_t *tokens, unsigned int num_tokens);

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_H_ */

