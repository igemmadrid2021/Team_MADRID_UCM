from Bio import SeqIO
from Bio.Seq import Seq
import re

#import_CDS() considers only continuous ORF. Exceptions like ribosomal slipagges are ignored.
def import_CDS(file):
    
    
    records = list(SeqIO.parse(file, "fasta")) #List with an ORF record in each position.
    ORFs = [] # Initilization of ORF start andend positions
    exception_flag = False # Exceptions or non-continuous ORFs are ignored
    
    for CDS in records: # Reading each CDS in the file
        
        # Analyzing headers of each CDS (description)
        
        
        #NON-STANDARD CDS CAN BE ALSO INCLUSDED UN-COMMENTING THESE LINES
        
        if re.search('exception',CDS.description):  # If exceptions are found, CDS is ignored
            exception_flag = True
            exception = re.search('\[(exception=[^\]]*)\]',CDS.description)
            print('One exception within the CDS file has been found "',exception.group(1),'" \n')
            
            # The moment so far the code only considers ribosomal slippages
            
            if re.search('join',CDS.description):        
                if re.search('complement',CDS.description):
                    strand = -1
                    localizaciones = re.search('.*\[location=<*complement\(.*\(<*([0-9]+).{2}>*([0-9]+),([0-9]+).{2}>*([0-9]+)\)\)\]',CDS.description)
                else:
                    strand = +1
                    localizaciones = re.search('.*\[location=<*.*\(<*([0-9]+).{2}>*([0-9]+),([0-9]+).{2}>*([0-9]+)\)\]',CDS.description)
            
            ORFs.append( (int(localizaciones.group(1))-1,int(localizaciones.group(2))-1,strand) )
        
        #Standard ORF analysis
        
        if exception_flag == False:
            if re.search('complement',CDS.description):
                strand = -1
                localizaciones = re.search('.*\[location=<*complement\(<*([0-9]+).{2}>*([0-9]+)\)\]',CDS.description)
            else:
                strand = +1
                localizaciones = re.search('.*\[location=<*([0-9]+).{2}>*([0-9]+)\]',CDS.description)
            
            # Start and stop codons positions are stored, as well as the coding strand (+1 / -1)
            ORFs.append( (int(localizaciones.group(1))-1,int(localizaciones.group(2))-1,strand) )
        
        exception_flag = False
    return ORFs


# search_convergent_NS() analyzes the position of different ORFs and looks for non-codifying regions between
# two convergent genes. Upstream of the sequence should be a CDS in the + strand while downstream should be a CDS in the - strand 
def search_convergent_NS(ORF_results,sequence,minimum_len,site_span,avoid_sites=None):
    
    """ 
    this function requires the following arguments:
        
       ORF_results: A tuple with ORF information (start,end, strand) 
       sequence: A Seq object corresponding with the sequence to search for neutral sites
       minimum_len: Minimum number of bases of region between convergent genes
       site_span: Number of bases of required full neutral site sequence
       avoid_sites: OPTIONAL ARGUMENT. Here can be introduced a dictionary whose keys should be an ID of a sequence to avoid
                      and its values the sequences (as Seq objects) to avoid in the neutral sites.
    
    """
    
    ORF_data = ORF_results #A list with ORF results is created 
    ORF_data.sort() # Results are sorted attending to their position
    orf_position = 0 #Initial variable for position of each ORF in the sorted list
    orf_number = len(ORF_data) # Number of identified ORFs
    convergent_NS=[] # Variable for storing positions of regions of interest
    NS_sequences = [] # Variable for storing the sequences of regions of interest
    NS_full_sequences = []
    NS_check = True # Boolean used for ignoring sites with sequences to avoid provided to the function as a dictionary
    NS_without_avoid_sites = 0

    # If there are any sequence to avoid they will be stored on a list for further use
    if avoid_sites != None:    
        avoid_keys = list(avoid_sites.keys())
        avoid_list = []
        for key in avoid_keys:
            avoid_list.append(avoid_sites[key])
            avoid_list.append(avoid_sites[key].reverse_complement())
    elif type(avoid_sites) != dict:
        print('avoid_sites parameter must be a dictionary\n\n dict_keys should be an identifier of the sequence\n dict_values should be a str object with the restriction site to avoid')
    print('Avoiding the following sequences: ')
    print(avoid_list,'\n')
    
    for orf in ORF_data: # Each ORF is analyzed and compared with following ORFs
        
        step = 0 # Step is the number of ORF ahead that are compared with the one under study
        strand = orf[2]   # Strand of the ORF is obtained      
 
        # As long as the end of the analyzed ORF is higher than the following ORF in the genome
        while orf[1] > ORF_data[orf_position+step][0]:
            if orf_position+step < (orf_number-1): # Limit to span only existing positions
                step +=1  # We will look in the next ORF until its start position is higher than the studied ORF end
            else:
                break # Once two consecutive non-overlaping ORF are identified program continues
        
        start = (orf[1]+1) # end position of the first ORF will define start of intergenic region
        end = (ORF_data[orf_position+step][0]-1) #start position of the second ORF will define end of intergenic region   
        
        # CONDITION OF CONVERGENT GENES. 
        # If the first ORF is in the + strand and the second in the - strand (convergent genes)
        if (ORF_data[orf_position+step][2]==-1) and (strand == 1) and (start-1 < end+1):
        
            # This line will search any region between CONVERGENT or DIVERGENT genes
            #if (ORF_data[orf_position+step][2] != strand) and (start-1 < end+1): 
              
            # We check if exists any other ORF between the identified convergent ORF pair
            
            for mid_orf in ORF_data: # Analysis of every ORF in the data
                span = range(mid_orf[0],mid_orf[1]) # Coverage of each ORF in the genome
                
                # Comparing start and end positions of every convergent region with the span of every ORF in the genome
                if start in span or end in span: 
                    #print('ORF solapante con otros')
                    NS_check = False # In case there exists an overlap the NS won't be stored
            
            if  end - start  > minimum_len and NS_check == True: #Length criteria. Only regions with enough length will be saved
                
                # DNA regions between convergent Open reading frames are saved 
                convergent_NS.append( (start,end,str(end-start) ) ) #Start, End and Length of the region
                NS_sequences.append( str(sequence[start:end]) ) # DNA sequence of the region as + strand
                
                #Once a site has been identified bases from adjacent genes are selected for defining homologous
                # recombination regions. 
                
                NS_site_span = site_span - (end-start) # Number of additional bases require for sufficient length of neutral site
                NS_up_seq = sequence[(start-int(NS_site_span/2)):(start)] # 1/2 of those bases are selected from upstream gene
                NS_down_seq = (sequence)[(end):(end+int(NS_site_span/2))] # 1/2 of those bases are selected from downstream gene
                full_NS = (NS_up_seq+str(sequence[start:end])+NS_down_seq)            
                
                if type(avoid_sites==dict): # Sequence to avoid are analyzed in the full Neutral site sequence in any posible orientation in (+) strand
                    for avoid_site in avoid_list: #Checking every restriction site introduced
                                              
                       if avoid_site in full_NS: 
                           
                           NS_check = False
        
                if NS_check == True: 
                    # If there is not any restriction site to avoid it will be saved
                    NS_full_sequences.append((NS_up_seq.lower()+str(sequence[start:end])+NS_down_seq.lower()))
                    NS_without_avoid_sites +=1 #Counter of sites without restriction sites on its sequence
                    
                else:
                    NS_full_sequences.append(0) ## If there one or more restriction sites to avoid 0 will be annexed to results list
        
        orf_position +=1 # Comparisson will start from the next ORF position (ignoring previous ones)
        NS_check = True # Boolean is restored to default
        
    print(len(convergent_NS),' Convergent potential Neutral Sites has been found with length > ',minimum_len,' bp')
    print('of which ',NS_without_avoid_sites,'sites does not feature restriction sites on adjacent genes regions being potential neutral site candidates')
     
    return (convergent_NS,NS_sequences,NS_full_sequences)

# export_NS() function will read the identified positions of NS and export them to a readable text file with sequence information     
def export_NS(NS_tuple,path):
    
    file_path = path+'\Convergent_NS_Results.txt'
    ns_file = open(file_path,'w')
    
    for position in range(len(NS_tuple[0])):
        
        site_seq = NS_tuple[1][position]
        site_info = NS_tuple[0][position]
        site_full_seq = NS_tuple[2][position]
    
    
        header_line = '>> CONVERGENT NON CODIFYING DNA   POSITION='+str(site_info[0])+' : '+str(site_info[1])+'\tlength='+str(site_info[2])+'\n'
        sequence_line = str(site_seq)+'\n\n'
        full_site_line = 'FULL NEUTRAL SITE SEQUENCE(bases from adjacent genes in lower caps):\n'+str(site_full_seq)+'\n__________________________________________\n\n'
        ns_file.write(header_line)
        ns_file.write(sequence_line)
        if site_full_seq == 0:
            ns_file.write('Not Shown. There Exists Restriction Sites In Adajacent Gene Regions'+'\n__________________________________________\n\n')
        else:
            ns_file.write(full_site_line)        
        
    ns_file.close()    


##IN CASE A GENEBANK CDS INFORMATION FILE IS NOT AVAILABLE.

# A function for identifying Open Reading Frames is defined. 
# Function outputs are Start and end positions in the + Strand, length, reading frame and codifying strand
# Identified ORFs could be nested one within each other. 



################################################################

##INIZIALIZATION OF THE PROGRAM. 
    
record = SeqIO.read("PCC_11801.fna",'fasta') #Loading de Sequence as SeqIO object.
CDS_file = "CDS_11801.txt" # Loading the GeneBank report with CDS information
genome = record.seq # Storing the sequence to analyze as a Seq object
trans_table_id = 11 # Definition of the NCBI codon usage table<

################################################################

         ###USING AVAILABLE CDS INFORMATION

ORFs = import_CDS(CDS_file)

# Dictionary with sequences to avoid in the Neutral sites

avoid_dict = {}
avoid_dict['BsmBI']=Seq('CGTCTC')
avoid_dict['BsaI']=Seq('GGTCTC')
avoid_dict['NotI']=Seq('GCGGCCGC')
avoid_dict['SapI']=Seq('GAAGAGC')

NS_min_len = 200 # Minimum length of the intergenic region between convergent genes
site_span = 1800 # Base pairs of the full integration region

neutral_sites = search_convergent_NS(ORFs,genome,NS_min_len,site_span,avoid_sites = avoid_dict )

export_NS(neutral_sites,'Results')


        ##PERFORMING A MANUAL EXPLORATION OF ORFs WITHIN THE GENOME 
        #Only ATG and GTG codons are considered as Start 
    
# min_len = 75 # Minimum length of the ORF in nt.

#ORF_results = find_orfs(genome, trans_table_id, min_len)

#export_ORFs(ORF_results,genome)
    
# neutral_sites = search_convergent_NS(ORF_results[0],genome,NS_min_len)

# export_NS(neutral_sites)


            

        

    
    


