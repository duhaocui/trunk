classdef sensmeas
    properties
        
    end
    methods
        function s = sensmeas(varargin)
            nvarargin = length(varargin);
            if nargin >=1
                if ~isempty( varargin{1} )
                    argnum = 1;
                    fnames = s.fieldnames;
                    while argnum<=nvarargin
                        if isa( varargin{argnum} , 'char')
                            ind = findincells( fnames, varargin(argnum) );
                            if ~isempty( ind )                                
                                if argnum + 1 <= nvarargin
                                    s = setfield( s, varargin{argnum} , varargin{argnum+1}(1));
                                    
                                    argnum = argnum + 1;
                                else
                                    error(sprintf('Value for the field %s has not been entered!', varargin{argnum} ));
                                end
                            elseif isa( varargin{argnum} , 'numeric')
                                % undecided course of action
                            end
                        end
                        argnum = argnum + 1;
                        
                    end
                elseif isempty(  varargin{1} )
                    s = s([]);
                end
            else
                % will return an untouched object s
            end
        end
        
        function d = getcatdim( these )
            namelist = these(1).fieldnames;
            d = 0;
            for i=1:length(namelist)
                d = d + length( getfield(these(1), namelist{i}) );
            end
            
        end
        function a = getcat( these )
            namelist = fieldnames( these(1) );
            
            indx = zeros(9,1);
            for i=1:length( namelist )
                if strcmp( namelist{i}, 'range' )
                    indx(1) = i;
                elseif strcmp( namelist{i}, 'bearing' )
                    indx(2) = i;
                elseif strcmp( namelist{i}, 'rangext' )
                    indx(3) = i;
                elseif strcmp( namelist{i}, 'bearingext' )
                    indx(4) = i;
                elseif strcmp( namelist{i}, 'bearingmeas' )
                    indx(5) = i;
                elseif strcmp( namelist{i}, 'linmeas' )
                    indx(6) = i;
                elseif strcmp( namelist{i}, 'mlinmeas' )
                    indx(7) = i;
                elseif strcmp( namelist{i}, 'pixmeas' )
                    indx(8) = i;
                elseif strcmp( namelist{i}, 'rssmeas' )
                    indx(9) = i;
                end
            end
     
            a = [];
            for i=1:length( indx )
                if indx( i )~= 0
                    a  = [a; cell2mat(eval(['{these.',namelist{indx(i)},'}']) )];            
                end
            end
            
        end
        
    end
end
