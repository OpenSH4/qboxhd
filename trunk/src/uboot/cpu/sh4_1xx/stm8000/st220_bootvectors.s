/* bootvector for st220-0 */
	.org	0x40,0
ENTRY(bootvector_lx0)
	.byte	 0x02, 0x00, 0x80, 0x15, 0xc0, 0x0f, 0x00, 0x88;
	.byte	 0xff, 0xff, 0x8f, 0x15, 0x80, 0xf0, 0x1f, 0x88;
	.byte	 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80;
	.byte	 0x3f, 0x00, 0x20, 0x06, 0xbf, 0xf0, 0x23, 0x81;
	.byte	 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80;
	.byte	 0x02, 0x00, 0x00, 0xb8, 0x00, 0x00, 0x00, 0xb1;
	.byte	 0x00, 0x00, 0x80, 0xb1;
/* bootvector for st220-1 */
	.org	0x80,0
ENTRY(bootvector_lx1)
	.byte	 0x04, 0x00, 0x80, 0x15, 0xc0, 0x0f, 0x00, 0x88;
	.byte	 0xff, 0xff, 0x8f, 0x15, 0x80, 0xf0, 0x1f, 0x88;
	.byte	 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80;
	.byte	 0x3f, 0x00, 0x20, 0x06, 0xbf, 0xf0, 0x23, 0x81;
	.byte	 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80;
	.byte	 0x02, 0x00, 0x00, 0xb8, 0x00, 0x00, 0x00, 0xb1;
	.byte	 0x00, 0x00, 0x80, 0xb1;
/* bootvector for st220-2 */
	.org	0xc0,0
ENTRY(bootvector_lx2)
	.byte	 0x06, 0x00, 0x80, 0x15, 0xc0, 0x0f, 0x00, 0x88;
	.byte	 0xff, 0xff, 0x8f, 0x15, 0x80, 0xf0, 0x1f, 0x88;
	.byte	 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80;
	.byte	 0x3f, 0x00, 0x20, 0x06, 0xbf, 0xf0, 0x23, 0x81;
	.byte	 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80;
	.byte	 0x02, 0x00, 0x00, 0xb8, 0x00, 0x00, 0x00, 0xb1;
	.byte	 0x00, 0x00, 0x80, 0xb1;