#! /bin/bash


#organization code start


replace_folders()
{
   if [ -d "$1" ] 
   then 
       for i in "$1"/*
       do
          replace_folders "$i" "$2" "$3"
       done
   elif [ -f  "$1" ]
   then 
       filename=$(basename "$1")
       extension="${filename##*.}"
       if [ "$extension" = "c" ]
       then
           mkdir "$2/C/$3"
           mv "$1" "$2/C/$3/main.c"
       elif [ "$extension" = "java" ]
       then
           mkdir "$2/Java/$3"
           mv "$1" "$2/Java/$3/Main.java"
       elif [ "$extension" = "py" ]
       then
           mkdir "$2/Python/$3"
           mv "$1" "$2/Python/$3/main.py"
       fi
   fi
       
   
}
organize_folders()
{

mkdir "$2"
for i in "$1"/*
do
       #echo "$i"
       foldername="${i:0: -4}"
       #echo "$foldername"
       newname="${i: -11: -4}"
       #echo $newname
       #mkdir "$foldername"
       unzip  -q "$i" -d "$2/$newname" 
        
       
done

mkdir "$2/C"
mkdir "$2/Python"
mkdir "$2/Java"

for i in "$2"/*
do 
    
    if ( [ "$i" != "$2/C" ] && [ "$i" != "$2/Python" ] && [ "$i" != "$2/Java" ] )
    then
       
        #name="${i: -7: -1}"
        name="$i/"
        name="${name: -8: -1}"
        replace_folders "$i" "$2" "$name"
        if [ -n "$3" ]
        then 
            if [ "$3" = "-v" ]
            then
               echo "Organizing files of "$name""
            fi
        fi
    
    fi
    
done


for i in "$2"/*
do 
    
    if ( [ "$i" != "$2/C" ] && [ "$i" != "$2/Python" ] && [ "$i" != "$2/Java" ] )
    then
       rm -r "$i"
    fi
    
done

}

#organization code finished

#execute code start

#C
#gcc main.c -o main.out
#./main.out < ../../../tests/test1.txt >output.txt

#Python
#cat ../../../tests/test1.txt | python3 main.py > output_file.txt


#java
#javac Main.java
#java Main  <../../../tests/test1.txt  >output.txt

execute_files()
{
  if [ -d "$1" ] 
   then 
       for i in "$1"/*
       do
          execute_files "$i" "$2" "$3"
       done
   elif [ -f  "$1" ]
   then 
       filename=$(basename "$1")
       extension="${filename##*.}"
       directoryName=$(dirname "$1")
       
       #echo $directoryName
       name=`echo "$directoryName" | cut -d '/' -f 3`
       #echo $name 
       if [ "$extension" = "c" ]
       then
           #echo "$filename"
           #echo "$directoryName"
           #echo "$1"
           gcc "$1" -o "$directoryName/"main.out
           if [ -n "$3" ]
           then 
             
              if [ "$3" = "-v" ]
              then
                 echo "Executing files of "$name""
              fi
           fi
           for j in "$2"/*
           do
             output="${j##*test}"
             #echo $output
             #output="${ouput%".txt"}"
             output="${output:0: -4}"
             
           ./"$directoryName/"main.out < "$j" > "$directoryName/"out"$output".txt
           done
           
       elif [ "$extension" = "java" ]
       then
           #echo "$filename"
           #echo "$1"
           javac "$1"
           if [ -n "$3" ]
           then 
             
              if [ "$3" = "-v" ]
              then
                 echo "Executing files of "$name""
              fi
           fi
           for j in "$2"/*
           do
             output="${j##*test}"
             #echo $output
             #output="${ouput%".txt"}"
             output="${output:0: -4}"
             java -cp "$directoryName" "Main"  < "$j"  > "$directoryName/"out"$output".txt
           done

       elif [ "$extension" = "py" ]
       then
          #echo "$filename"
          if [ -n "$3" ]
           then 
             
              if [ "$3" = "-v" ]
              then
                 echo "Executing files of "$name""
              fi
           fi
          for j in "$2"/*
           do
             output="${j##*test}"
             #echo $output
             #output="${ouput%".txt"}"
             output="${output:0: -4}"
             cat "$j" | python3 "$1" > "$directoryName/"out"$output".txt
          
           done
       fi
   fi
}

#execute code finished



#match code start

match_answer()
{  
   echo "student_id,type,matched,not_matched" > result.csv
   for i in "$1"/*
   do 
     for j in "$i"/*
     do 
        matched=0
        notmatched=0
        type=`echo "$j" | cut -d '/' -f 2`
        id=`echo "$j" | cut -d '/' -f 3`
        
        #echo $type
        #echo $id 
        for k in "$j"/*
        do
           
           file="${k##*out}"
           #echo $file
           #file="${file:0: -4}"
           for l in "$2"/*
           do
           answer="${l##*ans}"
           #echo $answer
           if [ "$file" = "$answer" ]
           then
             result=`diff -q "$k" "$l"`
             result=$?
            if [ $result = 1 ]
            then
                notmatched=`expr $notmatched + 1`
            else
                matched=`expr $matched + 1` 
            fi
           fi
           done
          
        done
        
        echo "$id,$type,$matched,$notmatched" >> result.csv
     done
   done
   mv result.csv "$1"/result.csv
}


#match code finish






#main
if [ -d "$2" ]
then
     rm -r "$2"
fi



if [ -z "$4" ]
then
    echo Usage:
    echo './organize.sh <submission folder> <target folder> <test folder> <answer folder> [-v] [-noexecute]'
    echo
    echo "-v: verbose"
    echo "-noexecute: do not execute code files"
else
   if [ -n "$5" ] && [ "$5" = "-v" ]
   then
       filecount=`ls "$3" | wc -l`
       echo "Found "$filecount" test files"
   fi
   organize_folders $1 $2 $5
   echo
   if [ -n "$5" ] && [ "$5" != "-noexecute" ] && [ -z "$6" ]   || [ -z "$5" ] 
      then
	 execute_files $2 $3 $5
	 match_answer $2 $4
   fi
   
   

fi








