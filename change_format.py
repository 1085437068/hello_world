#!/usr/bin/env python3
#--coding:utf-8--
#出现IOError可能与图片保存的新路径有关，换个路径试试
#出现Keyeeror由于fmt_output处多加了双引号
#在调试过程中发现os.path下的函数引用路径必须要有双引号，此程序在函数get_imlist中返回的数列已带有双引号！
from PIL import Image
import os
def get_imlist(path1,fmt_input):
	return[os.path.join(path1,f) for f in os.listdir(path1) if f.endswith(fmt_input)] #listdir显示所有文件名给f，返回所有输入格式的图片  f：文件名和path：路径

def convert_image_fmt(path1,path2,fmt_input,fmt_output):
	im_list=get_imlist(path1,fmt_input)
	for infile in im_list:  #将文件列表名字赋予infile
		file_name=os.path.basename(infile)  #抽出文件名
		outfile=path2+'/'+os.path.splitext(file_name)[0]+fmt_output #提取infile的名字赋予新的格式,fmt。。。不要用引号
		if os.path.splitext(infile)[1] != os.path.splitext(outfile)[1]: 
			try:
				Image.open(infile).save(outfile) #替换
			except IOError:
				print('cannot convert')
path1=input('Please enter path(original):')
path2=input('Please enter path(now):')
fmt_input=input('Please enter format(original):')
fmt_output=input('Please enter latter format(now):')
convert_image_fmt(path1,path2,fmt_input,fmt_output)


