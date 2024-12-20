#!/usr/bin/env python3

# note: apt-get install  python3-pycparser python3-polib

from pycparser import parse_file, c_ast
import polib
import sys

if(len(sys.argv)!=2):
	print("convertlang2po.py <lang/file>")
	sys.exit(1)

filename_pot='po/webalizer.pot'
filename_english='lang/webalizer_lang.english'
filename_lang=sys.argv[1]

ast_english = parse_file(filename_english, use_cpp=True)
ast_lang = parse_file(filename_lang, use_cpp=True)

#ast_english.show()


def tags_value(ast):
	t2v={}
	for decl in ast.ext:
		# strings are pointer on char, just check pointer for now :)
		# also values are always strings... So don't check.
		if(isinstance(decl.type, c_ast.PtrDecl)):
			#print(decl.name, "=", decl.init.value)
			t2v[decl.name]=decl.init.value.strip('"')
		# arrays
		elif(isinstance(decl.type, c_ast.ArrayDecl)):
			# of pointers ... assume chars
			if(isinstance(decl.type.type, c_ast.PtrDecl)):
				for idx, val in enumerate(decl.init.exprs):
					if(isinstance(val, c_ast.Constant)):
						key="{0}[{1}]".format(decl.name,idx)
						#print(key,"=",val.value)
						v=val.value.strip('"')
						if(v == 'May'):
							if(decl.name == 's_month'): v = 'short|May'
							if(decl.name == 'l_month'): v = 'long|May'
						t2v[key]=v
			# response case
			elif(decl.name=='response'):
				for idx, val in enumerate(decl.init.exprs):
					key="{0}[{1}]".format(decl.name,idx)
					#print(key,"=",val.exprs[0].value)
					t2v[key]=val.exprs[0].value.strip('"')
			# ctry case
			elif(decl.name=='ctry'):
				for idx, val in enumerate(decl.init.exprs):
					if(isinstance(val.exprs[1], c_ast.Constant)):
						if(isinstance(val.exprs[0], c_ast.Constant)):
							cc=val.exprs[0].value
						else:
							cc=''.join([c.value.strip("'") for c in val.exprs[0].args.exprs])
						key="{0}[{1}]".format(decl.name,cc)
						#print(key,"=",val.exprs[1].value)
						t2v[key]=val.exprs[1].value.strip('"')
			else:
				print("missed:", decl.name)
		else:
			print("missed:", decl.name)
	return t2v

tag2english = tags_value(ast_english);
tag2lang = tags_value(ast_lang);

po = polib.pofile(filename_pot)

#for entry in po:
#    print (entry.msgid, entry.msgstr)
for k in tag2english.keys():
	#print(k, tag2english[k], tag2lang[k])
	entry = po.find(tag2english[k])
	if entry:
		entry.msgstr = tag2lang[k]
		if 'fuzzy' in entry.flags:
			entry.flags.remove('fuzzy')

po.metadata['Language'] = tag2lang['langcode']
po.save('po/'+tag2lang['langcode']+'.po')
