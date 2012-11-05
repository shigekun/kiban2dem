cd 2_xml
for i in *.xml
do 
 ../xmldem $i 1
 mv *.tif ../3_demtif/
done
