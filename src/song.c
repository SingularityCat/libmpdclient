/* libmpdclient
   (c) 2003-2008 The Music Player Daemon Project
   This project's homepage is: http://www.musicpd.org

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Music Player Daemon nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <mpd/song.h>
#include "str_pool.h"

#include <assert.h>
#include <stdlib.h>

struct mpd_tag_value {
	struct mpd_tag_value *next;

	char *value;
};

struct mpd_song {
	struct mpd_tag_value tags[MPD_TAG_COUNT];

	/* length of song in seconds, check that it is not MPD_SONG_NO_TIME  */
	int time;
	/* if plchanges/playlistinfo/playlistid used, is the position of the
	 * song in the playlist */
	int pos;
	/* song id for a song in the playlist */
	int id;
};

struct mpd_song *mpd_song_new(void) {
	struct mpd_song *song = malloc(sizeof(*song));
	if (song == NULL)
		/* out of memory */
		return NULL;

	for (unsigned i = 0; i < MPD_TAG_COUNT; ++i)
		song->tags[i].value = NULL;

	song->time = MPD_SONG_NO_TIME;
	song->pos = MPD_SONG_NO_NUM;
	song->id = MPD_SONG_NO_ID;

	return song;
}

void mpd_song_free(struct mpd_song *song) {
	assert(song != NULL);

	for (unsigned i = 0; i < MPD_TAG_COUNT; ++i) {
		struct mpd_tag_value *tag = &song->tags[i], *next;

		if (tag->value == NULL)
			continue;

		str_pool_put(tag->value);

		tag = tag->next;

		while (tag != NULL) {
			assert(tag->value != NULL);
			str_pool_put(tag->value);

			next = tag->next;
			free(tag);
			tag = next;
		}
	}

	free(song);
}

struct mpd_song *
mpd_song_dup(const struct mpd_song *song)
{
	struct mpd_song *ret;
	bool success;

	assert(song != NULL);

	ret = mpd_song_new();
	if (ret == NULL)
		/* out of memory */
		return NULL;

	for (unsigned i = 0; i < MPD_TAG_COUNT; ++i) {
		const struct mpd_tag_value *src_tag = &song->tags[i];

		if (src_tag->value == NULL)
			continue;

		do {
			success = mpd_song_add_tag(ret, i, src_tag->value);
			if (!success) {
				mpd_song_free(ret);
				return NULL;
			}

			src_tag = src_tag->next;
		} while (src_tag != NULL);
	}

	ret->time = song->time;
	ret->pos = song->pos;
	ret->id = song->id;

	return ret;
}

bool
mpd_song_add_tag(struct mpd_song *song,
		 enum mpd_tag_type type, const char *value)
{
	struct mpd_tag_value *tag = &song->tags[type], *prev;

	if ((int)type < 0 || type == MPD_TAG_ANY || type >= MPD_TAG_COUNT)
		return false;

	if (tag->value == NULL) {
		tag->value = str_pool_get(value);
		if (tag->value == NULL)
			return false;
	} else {
		while (tag->next != NULL)
			tag = tag->next;

		prev = tag;
		tag = malloc(sizeof(*tag));
		if (tag == NULL)
			return NULL;

		tag->value = str_pool_get(value);
		if (tag->value == NULL) {
			free(tag);
			return false;
		}

		tag->next = NULL;
		prev->next = tag;
	}

	return true;
}

const char *
mpd_song_get_tag(const struct mpd_song *song,
		 enum mpd_tag_type type, unsigned idx)
{
	const struct mpd_tag_value *tag = &song->tags[type];

	if ((int)type < 0 || type == MPD_TAG_ANY || type >= MPD_TAG_COUNT)
		return NULL;

	if (tag->value == NULL)
		return NULL;

	while (idx-- > 0) {
		tag = tag->next;
		if (tag == NULL)
			return NULL;
	}

	return tag->value;
}

void
mpd_song_set_time(struct mpd_song *song, int t)
{
	song->time = t;
}

int
mpd_song_get_time(const struct mpd_song *song)
{
	return song->time;
}

void
mpd_song_set_pos(struct mpd_song *song, int pos)
{
	song->pos = pos;
}

int
mpd_song_get_pos(const struct mpd_song *song)
{
	return song->pos;
}

void
mpd_song_set_id(struct mpd_song *song, int id)
{
	song->id = id;
}

int
mpd_song_get_id(const struct mpd_song *song)
{
	return song->id;
}
