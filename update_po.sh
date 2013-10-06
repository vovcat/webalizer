# 7. Created webalizer.pot file with the follow command:
    xgettext --default-domain=webalizer --directory=. -o po/webalizer.pot \
             --language=C --add-comments="TRANS:" --escape --sort-by-file \
             --keyword=_ --keyword=N_ --keyword=Q_ --keyword=PL_:1,2 *.c *.h
# 8. Merged old .po files.
    for i in po/*.po;do echo -n $i;msgmerge -U $i po/webalizer.pot;done;
    rm -f po/*.po~;
