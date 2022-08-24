fn getSkinBones theNode = 
(
  local boneList = #()
  
  max modify mode
  local skinMod = theNode.modifiers[#Skin]

  if skinMod == undefined do 
  (
    messageBox ("No skin modifier for " + theNode.name) title: ""
    return undefined
  )

  modPanel.setCurrentObject skinMod
  local numBones = skinOps.getNumberBones skinMod
  for b = 1 to numBones do 
  (
    local boneName = skinOps.getBoneName skinMod b 1
    local boneNode = getNodeByName boneName
    append boneList boneNode
  )
  
  return boneList
)

fn _point val =
(
	if abs(val) < 0.0001 then return 0.0
	else return val
	)

fn _point3 p3 =
(
	rp3 = [0.0,0.0,0.0]
	if abs(p3.x)>0.0001 then rp3.x = p3.x
	if abs(p3.y)>0.0001 then rp3.y = p3.y
	if abs(p3.z)>0.0001 then rp3.z = p3.z
	return rp3
)

function exportXMA texFilter = 
(
  startFrame = animationRange.start
  endFrame   = animationRange.end
  currFrameRate  = frameRate 

  numObjects = 0
  for noode in rootnode.children do
  (
  	if classof noode == Editable_mesh and classof noode != Bone then 
    (
       numObjects += 1
    )
  )

  if numObject != 0 then 
  (
    	strFileName = getSaveFileName caption:"Save XMA file" types:"XMAnimation file *.xma|*.xma"
  		if strFileName == undefined then return "error: export aborted!"
  
  		file = fopen strFileName "wb"
  		if file == undefined then  return "error: could not create file!"
  )else return "Scene doesn't contain one or more objects!"

  WriteString file "#XMA FileFormat By Jatro 8D\n#XMA is an fileformat for XMEngine (c) Jatro!"
  WriteString file "LXMA19"      
	WriteShort file numObjects
	--format "%\n" numObjects
	
  for noode in rootnode.children do
  (
  	if classof noode == Editable_mesh and classof noode != Bone then 
    (  
      numVertices  = noode.numverts 
      numFaces     = getNumFaces  noode 
      numTVertices = getNumTVerts noode                                  

      _skin = noode.modifiers[#Skin]
  
      if _skin == undefined do 
      (
        messageBox ("No skin modifier for " + noode.name) title: ""
        return undefined
      )
      
      if GetCommandPanelTaskMode() != #modify do SetCommandPanelTaskMode mode:#modify     
      if ModPanel.GetCurrentObject() != noode.skin do ModPanel.SetCurrentObject noode.skin
      
      
      bone_list = #()
      bone_list = getSkinBones noode
      WriteShort file noode.name.count
      WriteString file noode.name
      
      WriteShort file numVertices
      WriteShort file numFaces
      WriteShort file bone_list.count
      WriteShort file startFrame
      WriteShort file endFrame
      WriteShort file currFrameRate

      for currVertex = 1 to numVertices do
      (           
  				local numBoneWeights = skinops.GetVertexWeightCount _skin currVertex
  				local maxWeight = 0.0
  				local maxBoneIdx = 0
  				
  				for weightIdx = 1 to numBoneWeights do
  				(
  					local w = skinops.GetVertexWeight _skin currVertex weightIdx
  					if w > maxWeight do
  					(
  						maxWeight = w
  						maxBoneIdx = skinops.GetVertexWeightBoneID _skin currVertex weightIdx
  					)
  				)
  			
        vertex = getVert noode currVertex
        WriteFloat file vertex.x 
        WriteFloat file vertex.y 
        WriteFloat file vertex.z 
        WriteLong file maxBoneIdx  
      )
      
      for currFace = 1 to numFaces do
      (                                                
  			   tface = getTVFace noode currFace
           for i=1 to 3 do
           (  
  				    index = tface[i]
  				    tvvert = getTVert noode index
              WriteFloat file tvvert.x 
              WriteFloat file tvvert.y 
           )
      )
  
      for currFace = 1 to numFaces do
      (                                                
  			   face = getFace noode currFace
  			   WriteLong file (face.x as integer - 1) #unsigned
  			   WriteLong file (face.z as integer - 1) #unsigned
  			   WriteLong file (face.y as integer - 1) #unsigned
      )
      
			if noode.material != undefined and noode.material.diffusemap != undefined then 
			(
        strTextureFileExploded = filterString noode.material.diffusemap.filename "\\."
	      strTextureFile = strTextureFileExploded[strTextureFileExploded.count - 3]+ "/" +strTextureFileExploded[strTextureFileExploded.count - 2]+ "/" +strTextureFileExploded[strTextureFileExploded.count - 1] + "." + strTextureFileExploded[strTextureFileExploded.count]
        iOpacity = noode.material.opacity / 100;
      )else(
        strTextureFile = "none.dds"
        iOpacity = 1.0
      )
          
      WriteShort file strTextureFile.count 
	    WriteString file strTextureFile
			WriteShort file (texFilter-1) 
      WriteFloat file iOpacity  
    
      for b = 1 to bone_list.count do       
      ( 
          _node = getnodebyname bone_list[b].name
          parent = _node.parent
                                                                                   
          index = b-1
          if parent == undefined then 
          (                       
            --format "% \"%\" -1\n" index bone_list[b].name
            WriteShort file -1
            
            WriteShort file bone_list[b].name.count
            WriteString file bone_list[b].name     
            
            WriteShort file 2
            WriteString file "-1"
            
          )else( 
            format "% \"%\" %\n" index bone_list[b].name parent.name    
            WriteShort file index
            
            WriteShort file bone_list[b].name.count
            WriteString file bone_list[b].name
            
            WriteShort file bone_list[index].name.count
            WriteString file bone_list[index].name
         ) 
     
          in coordsys parent tmppos = _node.pos        
          in coordsys parent tmprot = _node.rotation   
          format "% % %    % % %\n" tmppos.x tmppos.y tmppos.z tmprot.x tmprot.y tmprot.z
          
          WriteFloat file tmppos.x 
          WriteFloat file tmppos.y 
          WriteFloat file tmppos.z
          
          WriteFloat file tmprot.x 
          WriteFloat file tmprot.y 
          WriteFloat file tmprot.z
          
       )
       
      for currFrame = startFrame to endFrame do
      (
        at time currFrame
        (        
            --format "%\n" currFrame
            --WriteShort file currFrame      
            for b = 1 to bone_list.count do
            (                   
              _node = getnodebyname bone_list[b].name   
              parent = _node.parent
              
            	index = b-1
            	name  = skinops.getbonename _skin b 0
            	noode = getnodebyname name
            	
        	    rotation = [0.0,0.0,0.0]
            	in coordsys parent 
              (
            		postion  = _point3(_node.pos)
            		rotation = [_node.rotation.x_rotation, _node.rotation.y_rotation, _node.rotation.z_rotation]
            	)
            	
            	rotation.x = _point(degtorad rotation.x)
            	rotation.y = _point(degtorad rotation.y)
            	rotation.z = _point(degtorad rotation.z)
            
            	format "% % % % % % % %\n" index currFrame postion.x postion.y postion.z rotation.x rotation.y rotation.z
            	WriteShort file index
            	WriteFloat file currFrame
            	WriteFloat file postion.x
            	WriteFloat file postion.y
            	WriteFloat file postion.z
            	WriteFloat file rotation.x 
            	WriteFloat file rotation.y 
            	WriteFloat file rotation.z 
            )
        )
      )       
    )
  )
  fclose file
)
 
 
utility max2xma "Export *.XMA file" 
(               
	
	group "Texture filters"
	(
	  radiobuttons rbTexFilter labels:#("Linear","Bilinear","Mipmap Linear") default:3 
  )

  group "File export"
  (
  	button export "Export XMA File..."
  )
  	
	on export pressed do
	(             
    exportXMA rbTexFilter.state                    
  )	
)  