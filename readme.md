# Ambigram

Ambigram is a graph-based algorithm to reconstruct the possible BFB local haplotype during the evolution process. Ambigram currently support general WGS sequencing (PE), 10X linked-reads (10x), PacBio SMRT (PB) and Oxford Nanopore (ONT).

## Getting Started

To build Ambigram, run the following command:

```
cd /path/to/Ambigram/
mkdir build
cd build
cmake ..
make && make install
```

To see the option SpecHap support, run the following command:

```
localHap --op bfb --help
```

### Prerequisites
- [Cbc](https://github.com/coin-or/Cbc) solves integer linear programming.
- [htslib](https://github.com/samtools/htslib) is used for accessing common file formats, such as SAM, CRAM and VCF, used for high-throughput sequencing data.
- [SVAS](https://github.com/paprikachan/SVAS) integrates SEG and SV files into a .lh file, which is the standard input file of Ambigram.
- [SpecHap](https://github.com/deepomicslab/SpecHap) extracts barcodes from 10x data.
- [hpvpipe](https://github.com/panguangze/hpvpipe) generate JUNCS files by processing PB or ONT data.
- [Python3](https://www.python.org/downloads/)
- [samtools](https://github.com/samtools/samtools) deals with .bam files. 
- [pysam](https://pysam.readthedocs.io/en/latest/) counts the coverage of genomic positions by reads in region.

## Using Ambigram

### Data preprocessing

Ambigram supports various data types including Illumina pair-end (PE) reads, Oxford Nanopore (ONT) long reads, Pacific Biosciences (PB) long reads, 10x Genomics linked-reads with varying tumor purity and sequencing depth. The procedures and methods to process various data sets are available on the other GitHub page (https://github.com/deepomicslab/Onion-BFB_Paper). 

In general, we need two files: (1) SEG file that indicates genomic regions including chromosome names, start and end coordinates, and copy numbers; (2) SV file that consists of start and end chromosome names, break points, and strands as well as copy number of each SV junction. We provide a script for users to generate the SEG file by inputting a SV file and the corresponding BAM file based on SV break points and read depth. Then we need to run a script (SVAS/scripts/csv_sv.py) to convert the SV and SEG files into an LH file, which is the standard input file of our algorithm. Here is a set of sample input files:

* **example_seg.txt**

```
chr7:55281001-55282000	1
chr7:55282001-55283000	5
chr7:55283001-55284000	8
chr7:55284001-55285000	8
chr7:55285001-55286000	6
chr7:55286001-55287000	4
```

* **example_sv.txt**

```
chrom_5p	bkpos_5p	strand_5p	chrom_3p	bkpos_3p	strand_3p	avg_cn
chr7	55282001	-	chr7	55282001	+	2
chr7	55283001	-	chr7	55283001	+	1
chr7	55285000	+	chr7	55285000	-	1
```

* **example.lh**

```
SAMPLE example
AVG_CHR_SEG_DP 30
AVG_WHOLE_HOST_DP 30
AVG_JUNC_DP 30
PURITY 1
AVG_TUMOR_PLOIDY 2
PLOIDY 2m1
VIRUS_START 7
SOURCE 1
SINK 6
SEG H:1:chr7:55281001:55282000 15.0 1.0
SEG H:2:chr7:55282001:55283000 75.0 5.0
SEG H:3:chr7:55283001:55284000 120.0 8.0
SEG H:4:chr7:55284001:55285000 120.0 8.0
SEG H:5:chr7:55285001:55286000 90.0 6.0
SEG H:6:chr7:55286001:55287000 60.0 4.0
JUNC H:2:- H:2:+ 30.0 2.0 U B
JUNC H:3:- H:3:+ 15.0 1.0 U B
JUNC H:4:+ H:4:- 15.0 1.0 U B
JUNC H:1:+ H:2:+ 45.0 3.0 U B
JUNC H:2:+ H:3:+ 97.5 6.5 U B
JUNC H:3:+ H:4:+ 120.0 8.0 U B
JUNC H:4:+ H:5:+ 105.0 7.0 U B
JUNC H:5:+ H:6:+ 75.0 5.0 U B
```

To generate the SEG file from a SV file and the corresponding BAM file, run the following command:

```
python Ambigram/script/bfb_scripts.py generate_seg --sv_file=[path to SV file] --bam_file=[path to bam file] --wgs_depth=[whole genome average depth (default: 100)] --tumor_purity=[sample tumor purity (default: 1)]
```

To convert the SEG and SV files into an LH file, run the following command:

```
python Ambigram/script/bfb_scripts.py generate_lh --sv_fn=[path to SV file] --seg_fn=[path to SEG file] --sample=[output sample name]
```

Besides, we can input a JUNCS file that contains extra information from TGS data (10x, PB, and ONT), which may help Ambigram resolve more accurate BFB paths. A JUNCS file that comprises groups of segments, which are probably connected in order as patterns or loops on BFB paths. On the one hand, we can use BarcodeExtractor in SpecHap to get barcodes from 10x data and run the script (process_barcode.py) to generate the JUNCS file. On the other hand, we can use another script (hpvpipe/main.py process_tgs) to directly get the JUNCS file from PB or ONT data. Here is a sample JUNCS file:

* **example.juncs**

```
6+ 6- 5- 4- 3- 2- 2+
2- 2+ 3+ 4+ 5+ 6+ 6-
6+ 6- 5- 4- 3-
```

To generate a JUNCS file for 10x data ([SpecHap](https://github.com/deepomicslab/SpecHap) should be installed):

``` 
BarcodeExtract [path to 10x .bam file] [output path of .bed file]
python Ambigram/script/process_barcode --seg_file=[path to SEG file] --bed_file=[path to .bed file] --sample_name=[prefix of the output .juncs file]
```

To generate a JUNCS file for PB or ONT data ([hpvpipe](https://github.com/panguangze/hpvpipe) should be installed):

```
samtools bam2fq [path to PB/ONT .bam file] | seqtk seq -A > [output path of .fasta file]
python hpvpipe/main.py process_tgs -r [path to the reference genome] -l [path to .lh file] -t [path to .fasta file] -o [output path of .juncs file]
```

### Run Ambigram

Ambigram requires at least an LH file. To decipher BFB paths on each chromosome without translocation, run the following command:

``` 
localHap --op bfb --in_lh [path to your .lh file] --lp_prefix [sample name]
```

With a **JUNCS** file (extra information from TGS data), try:

``` 
localHap --op bfb --in_lh [path to your .lh file] --juncdb [path to your .juncs file] --junc_info true --lp_prefix [sample name]
```

If you have a very complicated sample, e.g. a sample with with high and various copy numbers, try **Sequential Mode** that will resolve a BFB path with length-decreasing components (without nested loops):

``` 
localHap --op bfb --in_lh [path to your .lh file] --juncdb [path to your .juncs file] --seq_mode true --lp_prefix [sample name]
```

We also provide an option to solve BFB paths from **single-cell** data. You can input **multiple LH files** that represents different CNV profiles evolving over time:

``` 
localHap --op bfb --in_lh [paths to your .lh files (seperated by ,), e.g., test1.lh,test2.lh,test3.lh] --lp_prefix [sample name]
```

To decipher BFB events with translocation, we just need to add one line (shown as following) that includes explicit options into the end of the LH file. Ambigram provides 4 complex cases involved with BFB and translocation. 

1. Insertion before BFB (e.g., segments of virus2 are inserted into chr1)

``` 
PROP I1:chr1:virus2:chr1
```

2. Insertion after BFB (e.g., segments of chr3 and chr10 are inserted into chr3, and insertion starts at segment3 or segment4 on chr3)

``` 
PROP I2:chr3:chr10:chr12 M:chr3 S:3:4
```

3. Concatenation before BFB (e.g., segments of chr7 are linked to virus3 by translocation)

``` 
PROP C1:chr7:virus3
```

4. Concatenation after BFB (e.g., segments of chr3 are concatenated with segments of chr6)

``` 
PROP C2:chr3:chr6
```

## Author

Ambigram is developed by DeepOmics lab under the supervision of Dr. Li Shuaicheng, City University of Hong Kong, Hong Kong, China.

xxx

## Built with

xxx

## License

xxx