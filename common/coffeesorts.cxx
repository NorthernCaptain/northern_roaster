#include "coffeesorts.hxx"
#include "debug.hxx"

#include "boost/format.hpp"

CoSorts::CoSorts()
{
    DBG(4, "CoffeeSorts::constructor - init data");
}


bool CoSorts::load_file(const std::string& fname)
{
    int    lines=0;
    char   buf[1024];

    FILE*  fp=fopen(fname.c_str(), "r");
    if(fp==0)
    {
	DBG(0, "CoSorts::load_file: ERROR: can't open file: " << fname);
	return false;
    }

    while(fgets(buf, 1000, fp))
    {
	lines++;
	if(buf[0]=='#') //
	    continue;
	if(buf[0]=='\r' || buf[0]=='\n')
	    continue;

	{
	    char short_name[128];
	    char full_name[256];
	    char *ptr=buf;
	    char *sptr=short_name;
	    char *fptr=full_name;

	    while(*ptr==' ') ptr++;
	    while(*ptr && *ptr!=' ' && *ptr!='\t')
		*sptr++ = *ptr++;
	    *sptr=0;
	    while(*ptr==' ' || *ptr=='\t') ptr++;
	    if(*ptr!='"')
	    {
		DBG(0, "CoSorts::load_file: wrong format at line " << lines);
		continue;
	    }
	    ptr++;
	    while(*ptr && *ptr!='"')
		*fptr++ = *ptr++;
	    *fptr=0;
	    if(short_name[0]==0 ||
	       full_name[0]==0)
	    {
		DBG(0, "CoSorts::load_file: not all elements in line " << lines);
	    }

	    SortMap::iterator it=data.find(short_name);
	    if(!add(short_name, full_name))
	    {
		DBG(0, boost::format("CoSorts::load_file: id=%s already exists, skipping")
		    % short_name);
		continue;
	    }
	    DBG(5, "CoSorts::load_file: item: [" << short_name << "]->[" << full_name << "]");
	}
    }

    fclose(fp);
    DBG(4, "CoSorts::load_file: loaded " << data.size() << " coffee sorts");
    return true;
}

bool CoSorts::save_file(const std::string& fname)
{
    FILE *fp=fopen(fname.c_str(), "w");
    if(fp==0)
    {
	DBG(0, "CoSorts::save_file: ERROR: can't save file: " << fname);
	return false;
    }
    
    fprintf(fp, "# This file is saved by northern roaster software\n");
    fprintf(fp, "# Beware that any contents will be overwritten\n");

    for(SortMap::iterator it=data.begin(); it!=data.end(); it++)
	fprintf(fp, "%-20s \"%s\"\n", it->first.c_str(), it->second->name.c_str());
    
    fclose(fp);
    return true;
}

bool CoSorts::add(const std::string& id, const std::string& name)
{
    SortMap::iterator it=data.find(id);
    if(it!=data.end())
	return false;
    data.insert(std::make_pair(id, new NameStr(name)));
    return true;
}

bool CoSorts::add(const std::string& id, const std::string& name, int qty)
{
    SortMap::iterator it=data.find(id);
    if(it!=data.end())
	return false;
    data.insert(std::make_pair(id, new NameStr(name, qty)));
    return true;
}
