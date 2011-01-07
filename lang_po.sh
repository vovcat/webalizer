#!/bin/bash
# create  lang.txt file with vars and strings from webalizer_lang.english file
cat lang/webalizer_lang.english | grep ^char | sed \
	-e 's/^char//' -e 's/ \*//' -e 's/= /=/' -e 's/ *=/=/' \
	-e 's/="/=/' -e 's/;$//' -e 's/"$//' -e 's/!/\\!/' | \
	grep -v "^l_month" | grep -v "^s_month" | grep -v "^h_msg" | \
	grep -v "^language=" | sort -r > lang.txt
# create a sed line with -e options that subst msg_* strings to _("string")
# take strings from lang.txt file.
SED_E="";
for i in $(cat lang.txt |cut -f1 -d=); do
  SUBST=$(cat lang.txt | grep ^$i=| cut -f2 -d=);
  SED_E="$SED_E -e "'"'"s/\<$i\>/_("'\"'$SUBST'\"'")/"'"'
done;
# create a script file with commands to subst and create a diff file
# I create this script because had errors when I tried to run commands here.
echo "" > lang_po_exec.sh
for i in *.c; do
  echo "cat $i | sed $SED_E > $i.new;" >> lang_po_exec.sh;
  echo "cat $i.new > $i;" >> lang_po_exec.sh;
  echo "rm -f $i.new;" >> lang_po_exec.sh;
done;
bash ./lang_po_exec.sh;
rm -f lang_po_exec.sh lang.txt;
