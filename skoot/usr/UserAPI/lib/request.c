/*
**	/usr/UserAPI/lib/request.c
**
**	Mainly for HTTPD (but possibly for any link-based system),
**	associate with request ID's a request URL and variables so
**	that e.g. HTML pages may be served with links containing
**	just that request ID -- if a link is clicked on, the data
**	associated with it is looked up here.
**
**	Copyright Skotos Tech Inc 1999
*/

# include <status.h>

# define ID_ROOF	((1 << 31) - 1)
# define HOUR		3600

inherit "/lib/womble";

mapping id_to_data;
int *hourly_id;
int id;
int call_handle;

/* atomic */ static void clean();
static void do_clean(int from, int to);

void womble_requests() {
    id_to_data = womble(id_to_data);
}

static
void create() {
   hourly_id = ({ 0 });
   id_to_data = ([ ]);

   clean();
}

#if 0
nomask
void patch() {
   call_handle = call_out("clean", random(120));
}
#endif

/* atomic */
int new_entry(mixed data) {
   if (++id >= ID_ROOF) {
      id = 0;
   }
   id_to_data[id] = data;
   while (map_sizeof(id_to_data) == status()[ST_ARRAYSIZE]) {
      clean();
   }
   if (!call_handle && map_sizeof(id_to_data) > 0) {
      call_handle = call_out("clean", HOUR);
   }
   return id;
}

int *query_hourly() {
   return hourly_id[..];
}

int *query_entries() {
   return map_indices(id_to_data);
}

mixed query_data(int id) {
   return id_to_data[id];
}

/* erase the last 24 hours' entries */

static
void clean() {
   int from, to, orig;

   if (call_handle) {
      remove_call_out(call_handle);
      call_handle = 0;
   }

   hourly_id += ({ id });
   if (sizeof(hourly_id) > 24) {
      /* wipe out the oldest entries */
      from = hourly_id[0];
      to   = hourly_id[1];

      orig = map_sizeof(id_to_data);

      if (from > to) {
	 /* it flipped over */
	 do_clean(from, ID_ROOF);
	 do_clean(0, to);
      } else {
	 do_clean(from, to);
      }
      Debug(ur_name(this_object()) + " cleaned an hour; # entries " + orig + " -> " + map_sizeof(id_to_data));
      hourly_id = hourly_id[1 ..];
   }
   if (map_sizeof(id_to_data) > 0) {
      call_handle = call_out("clean", HOUR);
   }
}

static
void do_clean(int from, int to) {
   int *idarr, i;

   idarr = map_indices(id_to_data[from .. to]);
   for (i = 0; i < sizeof(idarr); i ++) {
      id_to_data[idarr[i]] = nil;
   }
}
