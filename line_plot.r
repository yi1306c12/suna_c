csvname = commandArgs(trailingOnly=TRUE)[1]
xlab = commandArgs(trailingOnly=TRUE)[2]
ylab = commandArgs(trailingOnly=TRUE)[3]
ylim_lower = as.numeric(commandArgs(trailingOnly=TRUE)[4])
ylim_upper = as.numeric(commandArgs(trailingOnly=TRUE)[5])
table <- read.csv(csvname)/5


png(paste(csvname,".png",sep=""))

matplot(table,xlab="Number of Generations",ylab="Accumerated Reward",type="l",ylim=c(0,50))
#vec = 1:length(colnames(table))
#legend("bottomright",colnames(table),lty=vec,col=vec,bty="n")

dev.off()