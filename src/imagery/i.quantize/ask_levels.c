int 
ask_levels (void)
{
    char buf[100];
    int n;

    return 10;
#ifdef FOO
    while(1)
    {
	fprintf (stderr, "How many color levels? ");
	if (!G_gets(buf)) continue;
	if (sscanf (buf, "%d", &n) != 1) continue;
	if (n < 2)
	{
	    fprintf (stderr, "Must be at least 2\n");
	    continue;
	}
	if (n <= 10)
	    break;
	sprintf (buf, "This will result in a color table with %d entries. Ok? ",
		n*n*n);
	if (G_yes (buf, 0))
	    break;
    }
    return n;
#endif
}
