void con_exec()
{
	char* arg = strtok(NULL, " ");
	if (arg)
	{
		char tmp[256];
		char tmpvarname[128];
		sprintf(tmp, "%s.cfg", arg);
		FILE* f = fopen(tmp, "r");
		if (!f)
		{
			logprintf("Unable to exec file '%s'.", tmp);
		} else {
			while (fgets(tmp, 1024, f))
			{
				int len = strlen(tmp);
				if (len > 0 && tmp[len-1] == '\n') //if (len > 0 && tmp[0] == '\n')
					tmp[strlen(tmp)-1] = 0;
				len = strlen(tmp);
				if (len > 0 && tmp[len-1] == '\r')
					tmp[strlen(tmp)-1] = 0;
				if (tmp[0] == 0)
					continue;
				// If the line has a comment, finish it there.
				for (size_t i=0; i<strlen(tmp)-1; i++)
				{
					if (((tmp[i] == '/') && (tmp[i+1] == '/')) || tmp[i] == '#') // wtf
					{
						tmp[i] = 0;
						break;
					}
				}
				if (strlen(tmp) > 2)
				{
					if ((tmp[0] != '/') && (tmp[1] != '/'))
						for(int i = 0; tmp[i] != '\0'; i++)
						{
							if (tmp[i] == ' ')
							{
								tmpvarname[i] = '\0';
								break;
							}

							tmpvarname[i] = tmp[i];
						}
						if (pConsole->FindVariable(tmpvarname))
						{
							pConsole->Execute(tmp);
						}
				}
			}
			fclose(f);
		}
	} else {
		logprintf("Usage:");
		logprintf("  exec <filename>");
	}
}
