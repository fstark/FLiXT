# Hercules XT Video Player

This file contains random remarks, to be cleaned-up later.

## Image encoding

No brightness
No reverse video for the moment

## Format encoding

Here are the interesting modes for the HGC+

## Image encoding

No brightness
No reverse video for the moment

Here are the interesting modes for the HGC+

| Code | Name | Bytes per frame | Resolution | Char Resolution | Char Count | Description |
|------|------|----------------|------------|-----------------|------------|-------------|
| `f` | `fast` | 1890 | 90x21 | 8x16 | 256 | Fastest mode |
| `g` | ``good`` | 3780 | 90x21 | 8x16 | 3072 | Potentially a good compromise |
| `s` | ``standard`` | 3870 | 90x43 | 8x8 | 256 | Close to standard text mode, but 90 char width |
| `b` | ``best`` | 7740 | 90x43 | 8x8 | 3072 | Best quality, but a lot of data |

The most interesting mode for the HGC+ is the 3072 characters mode, ``good`` and ``best``, so we concentrate on getting a wire format that is adapted to those.

As we don't have reverse video, the pixel data consists of 12 bits.

|76543210|76543210|
|--------|--------|
|AAAAPPPP|PPPPPPPP|

Stats on typical data on ``best`` mode on the bullet time sequence of mnatrix:

* Video mem size: 7740 bytes (90\*43\*2)
* Frame char count: 3870
* Number of distinct char per screen: 362
* Changes during stable sequences: around 700 characters between two frames
* Changes during action sequences: around 2000 to 3000 characters, up to 3853 (ie: all chars)

As characters are allocated according to the whole video, the number of uniques is expected to grow if a video is shorter (and the quality will get higher). This will increase the size of the encoding.
