/*									tab:8
 *
 * photo.c - photo display functions
 *
 * "Copyright (c) 2011 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:	    Steve Lumetta
 * Version:	    3
 * Creation Date:   Fri Sep  9 21:44:10 2011
 * Filename:	    photo.c
 * History:
 *	SL	1	Fri Sep  9 21:44:10 2011
 *		First written (based on mazegame code).
 *	SL	2	Sun Sep 11 14:57:59 2011
 *		Completed initial implementation of functions.
 *	SL	3	Wed Sep 14 21:49:44 2011
 *		Cleaned up code for distribution.
 */


#include <string.h>

#include "assert.h"
#include "modex.h"
#include "photo.h"
#include "photo_headers.h"
#include "world.h"

#include <stdio.h>
#include <stdlib.h>
/* types local to this file (declared in types.h) */

/* 
 * A room photo.  Note that you must write the code that selects the
 * optimized palette colors and fills in the pixel data using them as 
 * well as the code that sets up the VGA to make use of these colors.
 * Pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of
 * the second row, and so forth.  No padding should be used.
 */
struct photo_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t        palette[192][3];     /* optimized palette colors */
    uint8_t*       img;                 /* pixel data               */
};

/* 
 * An object image.  The code for managing these images has been given
 * to you.  The data are simply loaded from a file, where they have 
 * been stored as 2:2:2-bit RGB values (one byte each), including 
 * transparent pixels (value OBJ_CLR_TRANSP).  As with the room photos, 
 * pixel data are stored as one-byte values starting from the upper 
 * left and traversing the top row before returning to the left of the 
 * second row, and so forth.  No padding is used.
 */
struct image_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t*       img;                 /* pixel data               */
};

struct octree_node_t
{
	/* data */
	unsigned int num_pixels;	// the node contains how much pixels
	unsigned long int r_sum;	// the red sum of pixels
	unsigned long int g_sum;	// green sum of pixels
	unsigned long int b_sum;	// blue sum of pixels
	uint16_t idx_level2;	// idx in level 2
	uint16_t idx_level4;	// idx in level 4
};

/* file-scope variables */

/* 
 * The room currently shown on the screen.  This value is not known to 
 * the mode X code, but is needed when filling buffers in callbacks from 
 * that code (fill_horiz_buffer/fill_vert_buffer).  The value is set 
 * by calling prep_room.
 */
static const room_t* cur_room = NULL; 


/* 
 * fill_horiz_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the leftmost 
 *                pixel of a line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- leftmost pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_horiz_buffer (int x, int y, unsigned char buf[SCROLL_X_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgx;  /* loop index over pixels in object image      */ 
    int            yoff;  /* y offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_X_DIM; idx++) {
        buf[idx] = (0 <= x + idx && view->hdr.width > x + idx ?
		    view->img[view->hdr.width * y + x + idx] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (y < obj_y || y >= obj_y + img->hdr.height ||
	    x + SCROLL_X_DIM <= obj_x || x >= obj_x + img->hdr.width) {
	    continue;
	}

	/* The y offset of drawing is fixed. */
	yoff = (y - obj_y) * img->hdr.width;

	/* 
	 * The x offsets depend on whether the object starts to the left
	 * or to the right of the starting point for the line being drawn.
	 */
	if (x <= obj_x) {
	    idx = obj_x - x;
	    imgx = 0;
	} else {
	    idx = 0;
	    imgx = x - obj_x;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_X_DIM > idx && img->hdr.width > imgx; idx++, imgx++) {
	    pixel = img->img[yoff + imgx];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/* 
 * fill_vert_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the top pixel of 
 *                a vertical line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- top pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_vert_buffer (int x, int y, unsigned char buf[SCROLL_Y_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgy;  /* loop index over pixels in object image      */ 
    int            xoff;  /* x offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_Y_DIM; idx++) {
        buf[idx] = (0 <= y + idx && view->hdr.height > y + idx ?
		    view->img[view->hdr.width * (y + idx) + x] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (x < obj_x || x >= obj_x + img->hdr.width ||
	    y + SCROLL_Y_DIM <= obj_y || y >= obj_y + img->hdr.height) {
	    continue;
	}

	/* The x offset of drawing is fixed. */
	xoff = x - obj_x;

	/* 
	 * The y offsets depend on whether the object starts below or 
	 * above the starting point for the line being drawn.
	 */
	if (y <= obj_y) {
	    idx = obj_y - y;
	    imgy = 0;
	} else {
	    idx = 0;
	    imgy = y - obj_y;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_Y_DIM > idx && img->hdr.height > imgy; idx++, imgy++) {
	    pixel = img->img[xoff + img->hdr.width * imgy];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/* 
 * image_height
 *   DESCRIPTION: Get height of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_height (const image_t* im)
{
    return im->hdr.height;
}


/* 
 * image_width
 *   DESCRIPTION: Get width of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_width (const image_t* im)
{
    return im->hdr.width;
}

/* 
 * photo_height
 *   DESCRIPTION: Get height of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_height (const photo_t* p)
{
    return p->hdr.height;
}


/* 
 * photo_width
 *   DESCRIPTION: Get width of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_width (const photo_t* p)
{
    return p->hdr.width;
}


/* 
 * prep_room
 *   DESCRIPTION: Prepare a new room for display.  You might want to set
 *                up the VGA palette registers according to the color
 *                palette that you chose for this room.
 *   INPUTS: r -- pointer to the new room
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes recorded cur_room for this file
 */
void
prep_room (const room_t* r)
{
    /* Record the current room. */
	photo_t* p = room_photo(r);
	fill_palette_mode_x_remaining(p->palette);
    cur_room = r;
}


/* 
 * read_obj_image
 *   DESCRIPTION: Read size and pixel data in 2:2:2 RGB format from a
 *                photo file and create an image structure from it.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the image
 */
image_t*
read_obj_image (const char* fname)
{
    FILE*    in;		/* input file               */
    image_t* img = NULL;	/* image structure          */
    uint16_t x;			/* index over image columns */
    uint16_t y;			/* index over image rows    */
    uint8_t  pixel;		/* one pixel from the file  */

    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the image pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (img = malloc (sizeof (*img))) ||
	NULL != (img->img = NULL) || /* false clause for initialization */
	1 != fread (&img->hdr, sizeof (img->hdr), 1, in) ||
	MAX_OBJECT_WIDTH < img->hdr.width ||
	MAX_OBJECT_HEIGHT < img->hdr.height ||
	NULL == (img->img = malloc 
		 (img->hdr.width * img->hdr.height * sizeof (img->img[0])))) {
	if (NULL != img) {
	    if (NULL != img->img) {
	        free (img->img);
	    }
	    free (img);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = img->hdr.height; y-- > 0; ) {

	/* Loop over columns from left to right. */
	for (x = 0; img->hdr.width > x; x++) {

	    /* 
	     * Try to read one 8-bit pixel.  On failure, clean up and 
	     * return NULL.
	     */
	    if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
		free (img->img);
		free (img);
	        (void)fclose (in);
		return NULL;
	    }

	    /* Store the pixel in the image data. */
	    img->img[img->hdr.width * y + x] = pixel;
	}
    }

    /* All done.  Return success. */
    (void)fclose (in);
    return img;
}
/* 
 * read_photo
 *   DESCRIPTION: Read size and pixel data in 5:6:5 RGB format from a
 *                photo file and create a photo structure from it.
 *                Code provided simply maps to 2:2:2 RGB.  You must
 *                replace this code with palette color selection, and
 *                must map the image pixels into the palette colors that
 *                you have defined.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the photo
 */
photo_t*
read_photo (const char* fname)
{
    FILE*    in;	/* input file               */
    photo_t* p = NULL;	/* photo structure          */
    uint16_t x;		/* index over image columns */
    uint16_t y;		/* index over image rows    */
    uint16_t pixel;	/* one pixel from the file  */
    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the photo pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (p = malloc (sizeof (*p))) ||
	NULL != (p->img = NULL) || /* false clause for initialization */
	1 != fread (&p->hdr, sizeof (p->hdr), 1, in) ||
	MAX_PHOTO_WIDTH < p->hdr.width ||
	MAX_PHOTO_HEIGHT < p->hdr.height ||
	NULL == (p->img = malloc 
		 (p->hdr.width * p->hdr.height * sizeof (p->img[0])))) {
	if (NULL != p) {
	    if (NULL != p->img) {
	        free (p->img);
	    }
	    free (p);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }
	struct octree_node_t arr_level_two[LEVEL_TWO_SIZE];	// array for level two
	struct octree_node_t arr_level_four[LEVEL_FOUR_SIZE];	// array for level four
	int arr_new_four[LEVEL_FOUR_SIZE];
	uint32_t i;	//counter
	uint32_t index;
	uint32_t index2;
	uint32_t photo_size = p->hdr.width*p->hdr.height;
	uint16_t pixel_data[photo_size];
/*	initialize the node in two octrees	*/
	initialize_octree(arr_level_four,LEVEL_FOUR_SIZE);
	initialize_octree(arr_level_two,LEVEL_TWO_SIZE);
    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = p->hdr.height; y-- > 0; ) {
	/* Loop over columns from left to right. */
	for (x = 0; p->hdr.width > x; x++) {
	    if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
		free (p->img);
		free (p);
	        (void)fclose (in);
		return NULL;
	    }
		pixel_data[(y * p->hdr.width) + x] = pixel;
		index = ret_r4g4b4(pixel);	//rrrrggggbbbb
		index2 = ret_r2g2b2(pixel);	//rrggbb
	
		arr_level_four[index].num_pixels++;
		arr_level_four[index].r_sum += (pixel>>11)& 0x001f;	//last 5 bits
		arr_level_four[index].g_sum += (pixel>>5) & 0x003f;	//keep 6 bits
		arr_level_four[index].b_sum += pixel & 0x001f;		// keep 5 bits
		arr_level_four[index].idx_level2 = index2;
		arr_level_four[index].idx_level4 = index;

		arr_level_two[index2].num_pixels++;
		arr_level_two[index2].r_sum += (pixel>>11)& 0x001f;	//last 5 bits
		arr_level_two[index2].g_sum += (pixel>>5) & 0x003f;	//keep 6 bits
		arr_level_two[index2].b_sum += pixel & 0x001f;		// keep 5 bits
	
    }
	}
	    (void)fclose (in);
	/*sort the octree nodes based on the number of pixels appearing in each and select the most
	frequent SORTED_LEVEL_SIZE for specific palette colors.	*/
	qsort(arr_level_four,LEVEL_FOUR_SIZE,sizeof(octree_node_t),cmpnodes);
	/* calculate the average color and write to the palette	*/

	for(i=0;i<SORTED_LEVEL_SIZE;i++){		//write SORTED_LEVEL_SIZE colors to the palette
		write_to_palette(p,arr_level_four,i,0);
		arr_new_four[arr_level_four[i].idx_level4] = i;	//bug here
		int index_lv2 = arr_level_four[i].idx_level2;
		arr_level_two[index_lv2].r_sum -= arr_level_four[i].r_sum;
		arr_level_two[index_lv2].g_sum -= arr_level_four[i].g_sum;
		arr_level_two[index_lv2].b_sum -= arr_level_four[i].b_sum;
		arr_level_two[index_lv2].num_pixels-= arr_level_four[i].num_pixels;	
	}
	for(i=SORTED_LEVEL_SIZE;i<LEVEL_FOUR_SIZE;i++){
		arr_new_four[arr_level_four[i].idx_level4] = arr_level_four[i].idx_level2+SORTED_LEVEL_SIZE;
	}
	/*	analyze the level 2	*/
	for(i=0;i<LEVEL_TWO_SIZE;i++){
		write_to_palette(p,arr_level_two,i,SORTED_LEVEL_SIZE);
	}
	for(i=0;i<photo_size;i++){
		pixel = pixel_data[i];
		index = arr_new_four[ret_r4g4b4(pixel)];
		p->img[i] = index+LEVEL_TWO_SIZE;
	}
    return p;
}
/* 
 * 	 cmpnodes
 *   DESCRIPTION: 
 *   INPUTS: const void* node1,node2
 *   OUTPUTS: none
 *   RETURN VALUE: if node1 >node2, return -1, else return 1, equal return 0
 *   SIDE EFFECTS: none
 */
int cmpnodes(const void* node1, const void* node2){
//	return (node1->num_pixels - node2->num_pixels);
	const octree_node_t* ocnode1 = node1;
	const octree_node_t* ocnode2 = node2;
	if(ocnode1->num_pixels > ocnode2->num_pixels)	{
		return -1;
	}
	else if(ocnode1->num_pixels == ocnode2->num_pixels){
		return 0;
	}
	else {
		return 1;
	}
	
}
/* 
 * 	 ret_r4g4b4
 *   DESCRIPTION: get the pixel's 12-bit representation
 *   INPUTS: pixel (16bits)
 *   OUTPUTS: none
 *   RETURN VALUE: 12-bit rrrrggggbbbb
 *   SIDE EFFECTS: none
 */
int ret_r4g4b4(int pixel){
	return ((pixel>>1) & 0xf)|(((pixel>>7)& 0xf)<<4)|(((pixel>>12)<<8));
}
/* 
 * 	 ret_r2g2b2
 *   DESCRIPTION: get the pixel's 6-bit representation
 *   INPUTS: pixel (16bits)
 *   OUTPUTS: none
 *   RETURN VALUE: 6-bit rrggbb
 *   SIDE EFFECTS: none
 */
int ret_r2g2b2(int pixel){
	return ((pixel>>3)&0x3)|(((pixel>>9)&0x3)<<2) | ((pixel>>14)<<4);
}
/* 
 * 	 initialize_octree
 *   DESCRIPTION: initialize the octree
 *   INPUTS: octree_node_t* arr_node-- array of the octreenode, int size-- size of array
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: change the tree node's attribute
 */
void initialize_octree(octree_node_t* arr_node, int size){
	int i;
	for(i = 0;i<size;i++){
		arr_node[i].num_pixels = 0;
		arr_node[i].r_sum = 0;
		arr_node[i].g_sum = 0;
		arr_node[i].b_sum = 0;
		arr_node[i].idx_level2=0;
		arr_node[i].idx_level4=0;
	}
}
/* 
 * 	 write_to_palette
 *   DESCRIPTION: write to the palette with the given
 *   INPUTS: octree_node_t* arr_node-- array of the octreenode, int size-- size of array
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: change the tree node's attribute
 */
void write_to_palette(photo_t* p,octree_node_t*arr_node,int i,int offset){
	int r_average, b_average, g_average;
	if(arr_node[i].num_pixels!=0){
		r_average = arr_node[i].r_sum/arr_node[i].num_pixels;
		b_average = arr_node[i].b_sum/arr_node[i].num_pixels;
		g_average = arr_node[i].g_sum/arr_node[i].num_pixels;
		p->palette[i+offset][0] = (r_average & 0x001f)<<1;	// get 6 bits due to the palette format
		p->palette[i+offset][1] = (g_average & 0x003f);
		p->palette[i+offset][2] = (b_average & 0x001f)<<1;
	}
	else{
		p->palette[i+offset][0] = 0;
		p->palette[i+offset][1] = 0;
		p->palette[i+offset][2] = 0;
	}
}


