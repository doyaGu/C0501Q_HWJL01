#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "json.h"
#include "parse_flags.h"

#ifdef TEST_FORMATTED
#define json_object_to_json_string(obj) json_object_to_json_string_ext(obj,sflags)
#else
/* no special define */
#endif


int json_test()
{
#if 0
	json_object *new_obj;
#ifdef TEST_FORMATTED
	int sflags = 0;
#endif

	MC_SET_DEBUG(1);

#ifdef TEST_FORMATTED
	sflags = parse_flags(argc, argv);
#endif

	new_obj = json_tokener_parse("/* more difficult test case */"
    "{ \"glossary\": { \"title\": \"example glossary\", \"GlossDiv\": { \"title\": \"S\", \"GlossList\": [ { \"ID\": \"SGML\", \"SortAs\": \"SGML\", \"GlossTerm\": \"Standard Generalized Markup Language\", \"Acronym\": \"SGML\", \"Abbrev\": \"ISO 8879:1986\", \"GlossDef\": \"A meta-markup language, used to create markup languages such as DocBook.\", \"GlossSeeAlso\": [\"GML\", \"XML\", \"markup\"] } ] } } }");
	printf("new_obj.to_string()=%s\n", json_object_to_json_string(new_obj));
	json_object_put(new_obj);
#else
    /*    {
      "name": "Brian",
      "sex": 0,
      "data": {
        "education": "master",
        "profession": "engineer"
      }
    }    */    struct json_object *root, *data;
    struct json_object *name, *sex, *edu, *prof; 
    // add to json
    root = json_object_new_object();
    json_object_object_add(root, "name", json_object_new_string("Brian"));
    json_object_object_add(root, "sex", json_object_new_int(0));
     
    data = json_object_new_object();
    json_object_object_add(data, "education", json_object_new_string("master"));
    json_object_object_add(data, "profession", json_object_new_string("engineer"));
    json_object_object_add(root, "data", data);
     
    // Output to string
    printf("%s \n", json_object_to_json_string(root));
         
    name = json_object_object_get(root, "name");
    sex = json_object_object_get(root, "sex");
    data = json_object_object_get(root, "data");
    // If parse fail, object is NULL
    if (data != NULL) {
      edu = json_object_object_get(data, "education");
      prof= json_object_object_get(data, "profession");
    }
     
    if (!name || !sex|| !edu || !prof) {
      printf("parse failed.");
      json_object_put(root);
      exit(-1);
    }
     
    // Fetch value
    printf("name=%s \n", json_object_get_string(name));
    printf("sex=%d \n", json_object_get_int(sex));
    printf("education=%s \n", json_object_get_string(edu));
    printf("profession=%s\n", json_object_get_string(prof));

#endif
	return EXIT_SUCCESS;
}
