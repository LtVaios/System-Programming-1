typedef struct set_struct* set;
typedef struct set_node* setnode;

set init_set(void);
void set_insert(set s, char new_link[1024]);
void delete_set(set s);
void write_all_to_file(set s, char* write_to);