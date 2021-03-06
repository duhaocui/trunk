classdef particle
    properties
        state
        weight
        label
    end
    methods 
        function p = particle(varargin)
            for cnt=1:2:length(varargin)
                if ischar( varargin{cnt})&& cnt +1 <= length(varargin )
                    if strcmp( lower( varargin{cnt} ), 'state' )
                        if isnumeric(varargin{cnt+1})
                            state_ = varargin{cnt+1};
                            if  sum(abs( size( state_ )- size( state_(:) )) )==0
                                p.state = state_(:);
                            elseif ndims(state_)==2
                                for j=1:size( state_, 2)
                                    p(j) = particle;
                                end
                                p = p.substates( state_ );
                            end
                        else
                            error('State should be a numeric array!');
                        end
                    elseif strcmp( lower( varargin{cnt} ), 'weight' )
                        if isnumeric(varargin{cnt+1})
                            weight_ =  varargin{cnt+1}(:);
                            for j=1:length(p)
                                p( j ).weight = weight_( j );
                            end
                        else
                            error('The weight should be a numeric array!');
                        end
                    elseif strcmp( lower( varargin{cnt} ), 'label' )
                        label_ = varargin{cnt+1};
                        if length( p) == 1
                            p.label  = { label_ };
                        else
                            for j=1:length(p)
                                p(j).label = label_{j};
                            end
                        end
                    else
                        warning(sprintf('Unidentified token: %s !', varargin{cnt}));
                    end
                else
                    error('Wrong input string');
                end
            end
        end
        
        function ind = getindx( particles, key )
            ind = [];
            for i=1:length(particles)
                labels_ = particles(i).label;
                for j=1:length(labels_)
                    label_ = labels_{j};
                   % if strcmp( num2str(label_), num2str(key) )
                   if isequal( label_, key )
                        ind = [ind,i];
                    end
                   
                end % for particle' s label
                
            end % for particle
        end % function ind = getindx( particles, key )
        function states = catstates(particles)
            states = zeros( length(particles(1).state), length(particles) );
            for i=1:length(particles)
                states(:,i) = particles(i).state(:);
            end
        end
        function outpars = substates( inpars, states )
            outpars = inpars;
            for i=1:size(states,2)
               outpars(i).state = states(:,i); 
            end
        end
        function weights = catweights(particles)
            weights = zeros( length(particles), 1 );
            for i=1:length(particles)
                weights(i) = particles(i).weight;
            end
        end
        function outpars = subweights( inpars, weights )
            outpars = inpars;
            for i=1:length(weights(:))
               outpars(i).weight = weights(i); 
            end
        end
        function labels_ = catlabels(particles)
            labels_ = cell( length(particles), 1 );
            for i=1:length(particles)
                labels_(i) = particles(i).label;
            end
        end
        function labels_ = getlastlabel(particles)
            labels_ = zeros( length(particles), 1 );
            for i=1:length(particles)
                labels_(i) = particles(i).label{end};
            end
        end
        function outpars = sublabels( inpars, labels )
            outpars = inpars;
            for i=1:length( inpars )
                if isa( labels, 'cell')
                    if length(labels)==1
                        outpars(i).label = labels(1);
                    else
                        outpars(i).label = labels(i); 
                    end
                else
                    if length(labels)==1
                        outpars(i).label = {labels(1) };
                    else
                        outpars(i).label = {labels(i)}; 
                    end
                end
            end
        end
        function outpars = addlabels( inpars, labels )
            outpars = inpars;
            for i=1:length( inpars )
                if isa( labels, 'cell')
                    outpars(i).label = [outpars(i).label, labels{i}]; 
                else
                    outpars(i).label = [outpars(i).label, {labels(i)}]; 
                end
            end
        end
        
        function c = mtimes(a,b)    
            if isa(a, 'particle') && isnumeric(b)
                if length(b)~=1
                   error('Vector multiplication is not defined. Try .* instead?'); 
                end
                c = a;
                for i=1:length(a)
                    c(i).weight = c(i).weight*b;
                end
                
            elseif isa(a, 'particle') && isa(b, 'particle')
               error('Not meaningful');
               
            elseif isa(b, 'particle') && isnumeric(a)
                c = b*a;
                return;
            end
        end % 
        function c = times(a, b)
            
            if abs( sum( size(a)-size(b)))~= 0
                error('In an expression a.*b, a and b should be of the same size.');
            end
            if isa( a, 'particle' )
                c = a;
            elseif isa( b, 'particle')
                c = b;
            end
            
            for i=1:length(a(:))
                c(i) = a(i)*b(i);
            end
        end % function c = times(a, b)
        function varargout = regularise( these )
            px = these.catstates;
            [nx Np] = size(px);
            xdim = nx;

            % this is nx=4 only
            A = (4/(nx+2))^(1/(nx+4));
            h_fact =  A * Np^(-1/(nx+4));
            C = cov(px');

            if rcond(C)>eps
                %Am = sqrtm(C);
                Am = chol(C)';
            else
                %disp('Problem with sqrtm in C matrix');
                Am = 0.00001*eye(xdim);
            end
            if ~isreal(Am)
                Am = 0.00001*eye(xdim);
            end
            gauss_mat = randn(nx,Np);

            py = px + 0.1*h_fact * Am *gauss_mat;


            regones = these;
            regones.substates( py );
            
                
            
            if nargout == 0
                if ~isempty( inputname(1) )
                    assignin('caller',inputname(1),regones);
                else
                    error('Could not overwrite the instance; make sure that the argument is not in an array!');
                end
            else
                varargout{1} = regones;
            end
        end % regones = regularise( these )
        function varargout = replabel( these, oldlabel, newlabel )
        
           regones = these;
           for i=1:length(regones)
              indx = findincells( regones( i ).label, {oldlabel} );
              if ~isempty( indx )
                  regones( i ).label{indx} = newlabel;               
              end
           end
            
           if nargout == 0
                if ~isempty( inputname(1) )
                    assignin('caller',inputname(1),regones);
                else
                    error('Could not overwrite the instance; make sure that the argument is not in an array!');
                end
            else
                varargout{1} = regones;
            end 
        end % replabel( these, oldlabel, newlabel )
        function g_ = par2gmm(these)
            % Find the corresponding gmm and return it back
            % first, find the cluster names
            c1 = these.getlastlabel;
            complabels = unique(c1,'legacy');
            numcomps = length( complabels );
            
            p1 = these.catstates;
            w1 = these.catweights;
            
            for i=1:numcomps
                ind_ = find( c1 == complabels( i ) );
                mean_ = mean( p1(:, ind_ ) ,2 );
                cov_ = cov( p1(:, ind_ )' );
                % Condition cov_
                [U,S,V] = svd(cov_);
                svals = diag(S);
                svals( find( svals < eps ) ) = eps;
                ccov_ = U*diag(svals)*V';
                pccov_ = (ccov_ + ccov_')/2;
                
                
                gks(i) = cpdf( gk(pccov_, mean_ ) );
                weights_(i) = sum(w1(ind_));
            end
            g_ = gmm( weights_, gks );
            
        end
        function draw(these, varargin)
            states_ = these.catstates;
            weights = these.catweights;
            
            dstates_ = [states_([1,2],:); weights' ];
            
            plotOpt = {'''b.'''};
            
            figureHandle = [];
            axisHandle = [];
            
            for cnt=1:2:length(varargin)
                if ischar( varargin{cnt})&& cnt +1 <= length(varargin )
                    if strcmp( lower( varargin{cnt} ), 'axis' )
                        axisHandle = varargin{cnt+1};
                        isAxisHandlesReceived = 1;
                    elseif strcmp( lower( varargin{cnt} ), 'figure' )
                        figureHandle =  varargin{cnt+1};
                    elseif strcmp( lower( varargin{cnt} ), 'options' )
                        plotOpt  = varargin{cnt+1};
                    else
                        warning(sprintf('Unidentified token: %s !', varargin{cnt}));
                    end
                else
                    error('Wrong input string');
                end
            end
            
            if isempty(axisHandle)
                if isempty(figureHandle)
                    figureHandle = figure;
                end
                axisHandle = gca;
            end
            
            axes(axisHandle);
            hold on
            grid on
            
            eval(['plot3(dstates_(1,:), dstates_(2,:), dstates_(3,:),', plotOpt{1},');'] )
            hold off
            
        end % draw(these, varargin)
    end % MEthods
end